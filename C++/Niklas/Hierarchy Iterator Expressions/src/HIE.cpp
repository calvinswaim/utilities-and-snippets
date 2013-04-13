/**
 * Simplified BSD License
 * Copyright (C) 2013, Niklas Rosenstein. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of the copyright
 * holders.
 *
 * ***********************************************************************
 *
 * HIE.h implementation.
 */

#include "HIE.h"

HIE_Container* HIE_Compiler::Compile(String input, HIE_ErrorLog* log) const {
    HIE_Container* container = new HIE_Container;
    if (!container) return NULL;

    HIE_BaseNode* node;
    HIE_InputScanner scanner(input);
    scanner.Read();

    while (!scanner.End()) {
        node = ReadNode(scanner, log);
        container->Push(node);
        if (log->IsFatal()) {
            break;
        }
    }

    // Deallocate the container on a fatal error.
    if (log->IsFatal()) {
        delete container;
        container = NULL;
    }

    return container;
}

HIE_BaseNode* HIE_Compiler::ReadNode(HIE_InputScanner& scanner,
                                     HIE_ErrorLog* log) const {
    if (scanner.End()) return NULL;
    char current = scanner.Chr();

    HIE_BaseNode* node = NULL;

    if (current == options.instr_Gopen) {
        scanner.Read();

        // Create the node-container and define it's mode.
        HIE_Container* container = new HIE_Container;
        if (!scanner.End()) {
            current = scanner.Chr();

            Bool skip = TRUE;
            if (current == options.instr_Gconsecutive) {
                container->mode = HIE_Container::MODE_CONSECUTIVE;
            }
            else if (current == options.instr_Gfirst) {
                container->mode = HIE_Container::MODE_FIRST;
            }
            else if (current == options.instr_Gaccum) {
                container->mode = HIE_Container::MODE_ACCUMULATE;
            }
            else {
                skip = FALSE;
            }
            if (skip) scanner.Read();
        }

        // Read the nodes for the group.
        while (!scanner.End() && scanner.Chr() != options.instr_Gclose) {
            node = ReadNode(scanner, log);
            if (!node) break;
            container->Push(node);
        }

        // If there was not already a fatal error, check if there should
        // be one.
        if (!log->IsFatal()) {
            if (scanner.End()) {
                HIE_Error error(HIE_EOI);
                log->Push(error);
                log->SetFatal();
            }
            else if (scanner.Chr() != options.instr_Gclose) {
                HIE_Error error(HIE_UNEXPECTEDCHARACTER, scanner.Chr(),
                                scanner.GetPosition());
                log->Push(error);
                log->SetFatal();
            }
        }

        node = container;
    }
    else if (current == options.instr_N) {
        node = new HIE_NextNode;
    }
    else if (current == options.instr_P) {
        node = new HIE_PredNode;
    }
    else if (current == options.instr_D) {
        node = new HIE_DownNode;
    }
    else if (current == options.instr_U) {
        node = new HIE_UpNode;
    }
    else if (current == options.instr_C && options.instr_C_supported) {
        node = new HIE_CacheNode;
    }
    else if (options.mode == HIE_CompilerOptions::MODE_STRICT) {
        HIE_Error error(HIE_UNEXPECTEDCHARACTER, current,
                        scanner.GetPosition());
        log->Push(error);
        log->SetFatal();
    }

    if (log->IsFatal()) {
        delete node;
        return NULL;
    }

    scanner.Read();
    if (scanner.End()) return node;

    // Check for the | operator.
    current = scanner.Chr();
    if (current == options.instr_Or) {
        scanner.Read();
        HIE_BaseNode* nextNode = ReadNode(scanner, log);
        if (!nextNode) {
            LONG position = scanner.GetPosition();
            HIE_Error error(HIE_EXPECTEDINSTRUCTION, position);
            log->Push(error);
            log->SetFatal();
            delete node;
            return NULL;
        }
        node = new HIE_OrOperatorNode(node, nextNode);
    }

    return node;
}

HIE_Container* HIE_CompileExpression(String input, HIE_Error* error,
            HIE_CompilerOptions* options) {
    HIE_Compiler compiler;
    if (options) compiler.options = *options;

    HIE_ErrorLog log;
    HIE_Container* node = compiler.Compile(input, &log);
    if (log.HasError() && error) {
        *error = log.GetLast();
    }

    return node;
}

