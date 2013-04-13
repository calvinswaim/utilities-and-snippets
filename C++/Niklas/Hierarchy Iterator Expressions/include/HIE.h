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
 * This header defines an interface for parsing and evaluating so called
 * "hierarchy iteration expressions". Such an HIE consists of characters
 * that define the way being travelled through a hierarchy of GeListNodes.
 */

#ifndef HIE_H
#define HIE_H

#include <c4d.h>
#include <ge_dynamicarray.h>
#include <cstring>

/**
 * This is the base class evaluating a single instruction in an
 * expression.
 */
class HIE_BaseNode {

    public:

    /**
     * Returns the next node relative to the passed node.
     * @param node The node that should be used for finding the next
     * node. Assumed to be not NULL.
     * @return The new node. May be NULL.
     */
    virtual GeListNode* GetNextNode(GeListNode* node) const = 0;

    /**
     * Destructor.
     */
    virtual ~HIE_BaseNode() {
    }

    /**
     * Allocator for a new instance. Overwritten for memory-management
     * purpose.
     */
    void* operator new (size_t size) {
        return GeAlloc(size);
    }

    /**
     * Deallocator for class instances. Overwritten for
     * memory-management purpose.
     */
    void operator delete (void* p) {
        GeFree(p);
    }

};

/**
 * This class evaluates the `N` instruction in an HIE, retrieving the
 * node next to the passed node.
 */
class HIE_NextNode : public HIE_BaseNode {

    public:

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        return node->GetNext();
    }

};

/**
 * This class evaluates the `P` instruction in an HIE, retrieving the
 * node preceding the passed node.
 */
class HIE_PredNode : public HIE_BaseNode {

    public:

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        return node->GetPred();
    }

};

/**
 * This class evaluates the `U` instruction in an HIE, retrieving the
 * parent of the passed node.
 */
class HIE_UpNode : public HIE_BaseNode {

    public:

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        return node->GetUp();
    }

};

/**
 * This class evaluates the `D` instruction in an HIE, retrieving the
 * passed nodes' first child.
 */
class HIE_DownNode : public HIE_BaseNode {

    public:

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        return node->GetDown();
    }

};

/**
 * THis class evaluates the `C` instruction in an HIE, retrieving the
 * cache of the passed node. It assumes the node be a BaseObject* instance.
 */
class HIE_CacheNode : public HIE_BaseNode {

public:

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        if (!node->IsInstanceOf(Obase)) return NULL;
        BaseObject* op = (BaseObject*) node;
        BaseObject* cache = op->GetDeformCache();
        if (!cache) cache = op->GetCache();
        return cache;
    }

};

/**
 * This node implements the OR operator for instructions.
 */
class HIE_OrOperatorNode : public HIE_BaseNode {

    /**
     * The left-hand node of the operator. Has precedance over
     * the right-hand node.
     */
    HIE_BaseNode* left;

     /**
      * The right-hand node of the operator.
      */
    HIE_BaseNode* right;

public:

    /**
     * Must be initialized with a left and right-hand node.
     * @param left The left hand instruction.
     * @param right The right hand instruction.
     */
    HIE_OrOperatorNode(HIE_BaseNode* left, HIE_BaseNode* right)
    : left(left), right(right) {}

    /**
     * Destructor.
     */
    virtual ~HIE_OrOperatorNode() {
        if (left) {
            delete left;
            left = NULL;
        }
        if (right) {
            delete right;
            right = NULL;
        }
    }

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        GeListNode* dest = left->GetNextNode(node);
        if (!dest) dest = right->GetNextNode(node);
        return dest;
    }

};

/**
 * This HIE_BaseNode implementation serves as a container for other
 * nodes.
 */
class HIE_Container : public HIE_BaseNode {
    /**
     * This array contains every node in the container being asked for
     * a next node.
     */
    GeDynamicArray<HIE_BaseNode*> nodes;

    public:

    /**
     * The mode for the container.
     */
    LONG mode;

    /**
     * This mode makes the container find the node using consecutive
     * application. It will resolve the passed GeListNode by passing it
     * to the first node, and passing the returned node to the next one.
     * NULL will be returned on the first hit.
     */
    static const LONG MODE_CONSECUTIVE = 0;

    /**
     * This mode makes the container try to find a node wherever
     * possible. When NULL is returned from a child-expression, the
     * last not-NULL node will be passed.
     */
    static const LONG MODE_ACCUMULATE = 1;

    /**
     * This mode returns the first node found by an expression.
     */
    static const LONG MODE_FIRST = 2;

    /**
     * Destructor.
     */
    virtual ~HIE_Container() {
        LONG count = nodes.GetCount();
        LONG index = 0;
        for (; index < count; index++) {
            delete nodes[index];
        }
        nodes.FreeArray();
    }

    /* Override: HIE_BaseNode */
    GeListNode* GetNextNode(GeListNode* node) const {
        LONG count = nodes.GetCount();
        LONG index = 0;
        GeListNode* dest = NULL;
        switch (mode) {
            case MODE_CONSECUTIVE:
                dest = node;
                for (; index < count && dest; index++)
                    dest = nodes[index]->GetNextNode(dest);
                break;
            case MODE_ACCUMULATE:
                for (; index < count && node; index++) {
                    dest = nodes[index]->GetNextNode(node);
                    if (dest) node = dest;
                }
                dest = node;
                break;
            case MODE_FIRST:
                for (; index < count; index++) {
                    dest = nodes[index]->GetNextNode(node);
                    if (dest) break;
                }
            default:
                #ifdef DEBUG
                    GeDebugOut("WARNING: Invalid mode for HIE_Container: "
                               "%d", mode);
                #endif
                break;
        }
        return dest;
    }

    /**
     * Add a node to the end of the container.
     * @param node The node to add.
     */
    void Push(HIE_BaseNode* node) {
        if (node)
            nodes.Push(node);
    }


};

/**
 * This class is utilized for scanning an input string character
 * by character.
 */
class HIE_InputScanner {

    /**
     * The input string.
     */
    char* input;

    /**
     * The length of the input string.
     */
    int inputLength;

    /**
     * The position in the input string.
     */
    int position;

    public:

    /**
     * Initialize the HIE_InputScanner from a Cinema 4D String
     * object.
     * @param string The string to initiaize the scanner with.
     */
    HIE_InputScanner(String string) : position(-1) {
        inputLength = string.GetLength();
        input = string.GetCStringCopy();
    }

    /**
     * Initialize the HIE_InputScanner from a native C-style
     * string.
     * @param string The C-style character array.
     * @param length The length of the input string. Pass a negative
     * number for automatic detection.
     */
    HIE_InputScanner(char* string, int length=-1) : position(-1) {
        if (length < 0)
            length = strlen(string);
        inputLength = length;
        input = (char*) GeAlloc(inputLength + 1);
        memcpy(input, string, inputLength);
        input[inputLength] = 0;
    }

    /**
     * Destructor.
     */
    ~HIE_InputScanner() {
        if (input) {
            GeFree(input);
            input = NULL;
        }
    }

    /**
     * Returns the current character in the input.
     * @return The current character.
     */
    char Chr() const {
        #ifdef DEBUG
            if (End()) {
                GeDebugOut("WARNING: Reading character from end of input "
                           "string.");
            }
        #endif
        return input[position];
    }

    /**
     * Jump to the next character in the input string.
     */
    void Read() {
        position++;
    }

    /**
     * @return TRUE when the scanner has it the EOI (end of
     * input) FALSE if there are character left to read.
     */
    Bool End() const {
        return position >= inputLength;
    }

    /**
     * @return TRUE when the scanner has not begun reading
     * yet, FALSE if not.
     */
    Bool Begin() const {
        return position == -1;
    }

    /**
     * @return The position in the input string.
     */
    int GetPosition() const {
        return position;
    }

    /**
     * @return The length of the input string.
     */
    int GetLength() const {
        return inputLength;
    }

};

/**
 * This class represents an error. It associates an error-code with
 * a data-type.
 */
class HIE_Error {

    public:

    /**
     * The error-code.
     */
    LONG code;

    /**
     * A union filled with the data for the error-code.
     */
    union {
        /**
         * For compilation errors, the position in the source.
         * HIE_EXPECTEDINSTRUCTION
         */
        LONG position;

        /**
         * HIE_UNEXPECTEDCHARACTER
         */
        struct {
            char got;
            LONG position;
        } e;
    } data;

    /**
     * Construct an unkown error.
     */
    HIE_Error() : code(-1) {
    }

    /**
     * HIE_EOI
     */
    HIE_Error(LONG code) : code(code) {
    }

    /**
     * Initialite with an error code and a value for the position.
     */
    HIE_Error(LONG code, LONG position) : code(code) {
        data.position = position;
    }

    /**
     * HIE_UNEXPECTEDCHARACTER
     */
    HIE_Error(LONG code, char got, LONG position) : code(code) {
        data.e.got = got;
        data.e.position = position;
    }

};

/**
 * This class is used for logging errors happened during the
 * process of compilation of an HIE.
 */
class HIE_ErrorLog {

    /**
     * An array of error-codes associated with the position
     * in the source.
     */
    GeDynamicArray<HIE_Error> errors;

    /**
     * TRUE when the last error was fatal, FALSE if not.
     */
    Bool fatal;

    public:

    /**
     * Initialize an empty error-log.
     */
    HIE_ErrorLog() : fatal(FALSE) {};

    /**
     * Flush all logged errors.
     */
    void Flush() {
        errors.FreeArray();
    }

    /**
     * Push an error to the array.
     */
    void Push(const HIE_Error& error) {
        errors.Push(error);
    }

    /**
     * Sets the error to fatal.
     */
    void SetFatal() {
        fatal = TRUE;
    }

    /**
     * @return TRUE when the last error was fatal, FALSE if not.
     */
    Bool IsFatal() const {
        return fatal;
    }

    /**
     * @return TRUE when the log has any errors.
     */
    Bool HasError() const {
        return errors.GetCount() != 0;
    }

    /**
     * @return The last error object.
     */
    HIE_Error GetLast() const {
        return errors[errors.GetCount() - 1];
    }

};

/**
 * Options for the HIE_Compiler.
 */
class HIE_CompilerOptions {

    public:

    /**
     * Strict mode for compilation. Used as the default value. Does
     * not allow unkown characters in the source.
     */
    static const LONG MODE_STRICT = 0;

    /**
     * Mode for just skipping any unkown characters.
     */
    static const LONG MODE_LOOSE = 1;

    /**
     * The `N` instruction.
     */
    char instr_N;

    /**
     * The `P` instruction.
     */
    char instr_P;

    /**
     * The `D` instruction.
     */
    char instr_D;

    /**
     * The `U` instruction.
     */
    char instr_U;

    /**
     * The `C` instruction.
     */
    char instr_C;

    /**
     * Whether the `C` instruction is supported or not.
     */
    Bool instr_C_supported;

    /**
     * The '(' instruction.
     */
    char instr_Gopen;

    /**
     * The ')' instruction.
     */
    char instr_Gclose;

    /**
     * The '|' instruction.
     */
    char instr_Or;

    /**
     * Instruction for making a group explicitly consecutive.
     */
    char instr_Gconsecutive;

    /**
     * Instruction for making a group return the first found node.
     */
    char instr_Gfirst;

    /**
     * Instruction for making a group accumulative.
     */
    char instr_Gaccum;

    /**
     * The compilation mode.
     */
    LONG mode;

    /**
     * Initializes the HIE_CompilerOptions object with the default
     * values.
     */
    HIE_CompilerOptions() {
        instr_N = 'N';
        instr_P = 'P';
        instr_D = 'D';
        instr_U = 'U';
        instr_C = 'C';
        instr_C_supported = TRUE;
        instr_Gopen = '(';
        instr_Gclose = ')';
        instr_Or = '|';
        instr_Gconsecutive = '!';
        instr_Gfirst = '?';
        instr_Gaccum = '~';
        mode = MODE_STRICT;
    }

};

/**
 * This class parses an HIE input expression and builds a node
 * structure for evaluating it.
 */
class HIE_Compiler {

    public:

    /**
     * Options for the compiler.
     */
    HIE_CompilerOptions options;

    /**
     * Initialize the compiler with the default options.
     */
    HIE_Compiler() {}

    /**
     * Initialize the compiler with the passed compiler
     * options.
     * @param options The options for the compiler.
     */
    HIE_Compiler(const HIE_CompilerOptions& options) : options(options) {}

    /**
     * Destructor.
     */
    ~HIE_Compiler() {}

    /**
     * Parse an input expression and returns a HIE_Container pointer.
     * Errors are saved in the HIE_ErrorLog instance.
     * @param string The input string with the HIE expression.
     * @param log Instance for logging errors.
     * @return A pointer to a HIE_Container node, or NULL if the
     * compilation failed.
     */
    HIE_Container* Compile(String input, HIE_ErrorLog* log) const;

    private:

    /**
     * Reads in a node at the current place.
     */
    HIE_BaseNode* ReadNode(HIE_InputScanner& scanner, HIE_ErrorLog* log) const;

};

/**
 * Standart error codes.
 */
enum {
    HIE_EOI,
    HIE_EXPECTEDINSTRUCTION,
    HIE_UNEXPECTEDCHARACTER,
};

/**
 * Compile an expression with the default options.
 * @param input The input expression.
 * @param error Assigned the HIE_Error object when occured.
 * @return A pointer to a container node. NULL on failure.
 */
HIE_Container* HIE_CompileExpression(String input, HIE_Error* error,
            HIE_CompilerOptions* options=NULL);

#endif /* HIE_H */



