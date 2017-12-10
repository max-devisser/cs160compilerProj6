#include "codegeneration.hpp"

void gen(std::string str) {
	std::cout << str << std::endl;
}

template<typename... Args>
void gen(std::string str, Args... args) {
	gen(str);
	gen(args...);
}

// CodeGenerator Visitor Functions: These are the functions
// you will complete to generate the x86 assembly code. Not
// all functions must have code, many may be left empty.

void CodeGenerator::visitProgramNode(ProgramNode* node) {
	gen(
		".data",
		"printstr: .asciz \"%d\\n\"",
		".text",
		".globl Main_main"
	);

	node->visit_children(this);
}

void CodeGenerator::visitClassNode(ClassNode* node) {
	currentClassName = node->identifier_1->name;
	currentClassInfo = classTable->at(currentClassName);
	node->visit_children(this);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    currentMethodName = node->identifier->name;
    currentMethodInfo = classTable->at(currentClassName).methods->at(currentMethodName);

    gen(
    	"# visitMethodNode",
    	currentClassName + "_" + currentMethodName + ":"
    );

    node->visit_children(this);
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
    gen(
    	"# visitMethodBodyNode",
    	"push %ebp",
    	"mov %esp, %ebp",
    	"sub $" + std::to_string(currentMethodInfo.localsSize) + ", %esp",
    	"push %ebx",
    	"push %esi",
    	"push %edi"
    );


    node->visit_children(this);

    // if (currentMethodName == currentClassName) {
    // 	gen(
    // 		"mov 8(%ebp), %eax"
    // 	);
    // }

    gen(
    	"pop %edi",
    	"pop %esi",
    	"pop %ebx",
    	"mov %ebp, %esp",
    	"pop %ebp",
    	"ret"
    );


}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
    node->visit_children(this);

	gen("# visitParameterNode");
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
    node->visit_children(this);

	gen("# visitDeclarationNode");
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
	node->visit_children(this);

	gen(
		"# visitReturnStatementNode",
		"pop %eax"
	);
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    node->visit_children(this);

    gen("# visitAssignmentNode");

    int offset = 0;
    if (node->identifier_2) {
    	offset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;  	
    }

    if (currentMethodInfo.variables->count(node->identifier_1->name)) {
		offset += currentMethodInfo.variables->at(node->identifier_1->name).offset;
	} else {
		offset += 8 + currentClassInfo.members->at(node->identifier_1->name).offset;
	}

	gen(
		"pop %eax",
		"mov %eax, " + std::to_string(offset) + "(%ebp)"
	);
}

void CodeGenerator::visitCallNode(CallNode* node) {
    node->visit_children(this);
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
    gen(
		"# visitIfElseNode"
 	);

    node->expression->accept(this);

    std::string currentLabel = std::to_string(nextLabel());

    gen(
		"pop %eax",
		"cmp $0, %eax",
		"je else" + currentLabel
	);

	if (node->statement_list_1) {
		for(std::list<StatementNode*>::iterator iter = node->statement_list_1->begin();
			iter != node->statement_list_1->end(); iter++) {
			(*iter)->accept(this);
		}
	}
	
	gen(
		"jmp end" + currentLabel,
		"else" + currentLabel + ":"
	);


	if (node->statement_list_2) {
		for(std::list<StatementNode*>::iterator iter = node->statement_list_2->begin();
			iter != node->statement_list_2->end(); iter++) {
			(*iter)->accept(this);
		}
	}

	gen(
		"end" + currentLabel + ":"
	);
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
    std::string currentLabel = std::to_string(nextLabel());
	gen(
		"# visitWhileNode",
		"loopstart" + currentLabel + ":"
 	);

	node->expression->accept(this);

	gen(
		"pop %eax",
		"cmp $0, %eax",
		"je loopend" + currentLabel		
	);

	if (node->statement_list) {
		for(std::list<StatementNode*>::iterator iter = node->statement_list->begin();
			iter != node->statement_list->end(); iter++) {
			(*iter)->accept(this);
		}
	}

	gen(
		"jmp loopstart" + currentLabel,
		"loopend" + currentLabel + ":"
	);
}

void CodeGenerator::visitPrintNode(PrintNode* node) {
   node->visit_children(this);

   gen(
   		"# visitPrintNode",
   		"push $printstr",
   		"call printf",
   		"add $8, %esp"
   );
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
	std::string currentLabel = std::to_string(nextLabel());
	gen(
		"# visitDoWhileNode",
		"loopstart" + currentLabel + ":"
 	);

	node->visit_children(this);

	gen(
		"pop %eax",
		"cmp $1, %eax",
		"je loopstart" + currentLabel		
	);
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
    node->visit_children(this);

	gen(
		"# visitPlusNode",
		"pop %eax",
		"pop %ebx",
		"add %ebx, %eax",
		"push %eax"
	);
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
	node->visit_children(this);

	gen(
		"# visitMinusNode",
		"pop %ebx",
		"pop %eax",
		"sub %ebx, %eax",
		"push %eax"
	);
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
	node->visit_children(this);

	gen(
		"# visitTimesNode",
		"pop %eax",
		"pop %ebx",
		"imul %ebx, %eax",
		"push %eax"
	);
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
	node->visit_children(this);

	gen(
		"# visitDivideNode",
		"pop %ebx",
		"pop %eax",
		"cdq"
		"idiv %ebx",
		"push %eax"
	);
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
	node->visit_children(this);

	std::string currentLabel = std::to_string(nextLabel());
	gen(
		"# visitGreaterNode",
		"pop %eax",
		"pop %ebx",
		"cmp %ebx, %eax",
		"jg greater" + currentLabel,
		"mov $0, %eax",
		"jmp done" + currentLabel,
		"greater" + currentLabel + ":",
		"mov $1, %eax",
		"done" + currentLabel + ":",
		"push %eax"
	);
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
	node->visit_children(this);

	std::string currentLabel = std::to_string(nextLabel());
	gen(
		"# visitGreaterEqualNode",
		"pop %eax",
		"pop %ebx",
		"cmp %ebx, %eax",
		"jge greaterequal" + currentLabel,
		"mov $0, %eax",
		"jmp done" + currentLabel,
		"greaterequal" + currentLabel + ":",
		"mov $1, %eax",
		"done" + currentLabel + ":",
		"push %eax"
	);
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
	node->visit_children(this);

	std::string currentLabel = std::to_string(nextLabel());
	gen(
		"# visitEqualNode",
		"pop %eax",
		"pop %ebx",
		"cmp %ebx, %eax",
		"je equal" + currentLabel,
		"mov $0, %eax",
		"jmp done" + currentLabel,
		"equal" + currentLabel + ":",
		"mov $1, %eax",
		"done" + currentLabel + ":",
		"push %eax"
	);
}

void CodeGenerator::visitAndNode(AndNode* node) {
	node->visit_children(this);

	gen(
		"# visitAndNode",
		"pop %eax",
		"pop %ebx",
		"and %ebx, %eax",
		"push %eax"
	);
}

void CodeGenerator::visitOrNode(OrNode* node) {
	node->visit_children(this);

	gen(
		"# visitOrNode",
		"pop %eax",
		"pop %ebx",
		"or %ebx, %eax",
		"push %eax"
	);
}

void CodeGenerator::visitNotNode(NotNode* node) {
	node->visit_children(this);

	gen(
		"# visitNotNode",
		"mov $1, %ebx",
		"pop %eax",
		"xor %ebx, %eax",
		"push %eax"
	);
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
	node->visit_children(this);

	gen(
		"# visitNegationNode",
		"pop %eax",
		"neg %eax",
		"push %eax"
	);
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
    node->visit_children(this);

    gen("# visitMethodCallNode");

    int numArgs = 0;
    if (node->expression_list) {
   		numArgs = node->expression_list->size();
	}

	int offset = 0;
	std::string className = "";
	std::string methodName = "";

	if (!node->identifier_2) {
		methodName = node->identifier_1->name;
		className = currentClassName;
		while(className.compare("")) {
			className = classTable->at(className).superClassName;
		}

		offset += 8;
	} else {
		methodName = node->identifier_2->name;
		className = node->identifier_1->objectClassName;
		while(className.compare("")) {
			className = classTable->at(className).superClassName;
		}

		if (currentMethodInfo.variables->count(node->identifier_1->name)) {
			offset += currentMethodInfo.variables->at(node->identifier_1->name).offset;
		} 
		else {
			offset += 8 + currentClassInfo.members->at(node->identifier_1->name).offset;
		}
	}

	gen(
		"push " + std::to_string(offset) + "(%ebp)",
		"call " + className + "_" + methodName,
		"add $" + std::to_string(4 * (numArgs + 1)) + ", %esp",
		"push %eax"
	);
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    node->visit_children(this);

    gen("# visitMemberAccessNode");

    int offset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;
    if (currentMethodInfo.variables->count(node->identifier_1->name)) {
    	offset += currentMethodInfo.variables->at(node->identifier_1->name).offset;
    } else {
    	offset += 8 + currentClassInfo.members->at(node->identifier_1->name).offset;
    }

    gen(
    	"mov " + std::to_string(offset) + "(%ebp), %eax",
    	"push %eax"
    );
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
    node->visit_children(this);

    gen("# visitVariableNode");
    
    if (currentMethodInfo.variables->count(node->identifier->name)) {
    	// local variable
    	gen("push " + 
    		std::to_string(currentMethodInfo.variables->
    			at(node->identifier->name).offset) + 
    		"(%ebp)"
    	);
    } else {
    	// member variable
    	gen(
    		"mov 8(%ebp), %ebx",
    		"mov " + std::to_string(currentClassInfo.members->
    			at(node->identifier->name).offset) +
    			"(%ebx), %eax",
    		"push %eax"
    	);
    }
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    node->visit_children(this);

    gen("# visitIntegerLiteralNode",
    	"push $" + std::to_string(node->integer->value)
    );
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    node->visit_children(this);

    gen("# visitBooleanLiteralNode",
    	"push $" + std::to_string(node->integer->value)
    );
}

void CodeGenerator::visitNewNode(NewNode* node) {
    node->visit_children(this);

    std::string objectSize = std::to_string(classTable->at(node->identifier->name).membersSize);

    gen(
    	"# visitNewNode",
    	"push $" + objectSize,
    	"call malloc",
   		"add $4, %esp",
   		"push %eax"
   	);

    if (classTable->at(node->identifier->name).methods->count(node->identifier->name)) {
    	int numArgs = 0;
    	if (node->expression_list) {
    		numArgs = node->expression_list->size();
		}

		gen(
			"call " + node->identifier->name + "_" + node->identifier->name,
			"add $" + std::to_string(4 * (numArgs + 1)) + ", %esp",
			"push %eax"
		);
    }
}

void CodeGenerator::visitIntegerTypeNode(IntegerTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitBooleanTypeNode(BooleanTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitObjectTypeNode(ObjectTypeNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitNoneNode(NoneNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIdentifierNode(IdentifierNode* node) {
    // WRITEME: Replace with code if necessary
}

void CodeGenerator::visitIntegerNode(IntegerNode* node) {
    // WRITEME: Replace with code if necessary
}
