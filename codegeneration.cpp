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
		"",
		" # Begin Program Node"
	);

	node->visit_children(this);

	gen(
		" # End Program Node"
	);
}

void CodeGenerator::visitClassNode(ClassNode* node) {
	gen(
		" # Begin Class Node: " + node->identifier_1->name
	);

	currentClassName = node->identifier_1->name;
	currentClassInfo = classTable->at(currentClassName);
	node->visit_children(this);

	gen(
		" # End Class Node: " + node->identifier_1->name
	);
}

void CodeGenerator::visitMethodNode(MethodNode* node) {
    currentMethodName = node->identifier->name;
    currentMethodInfo = classTable->at(currentClassName).methods->at(currentMethodName);

    gen(
    	" # Begin Method Node: " + currentMethodName,
    	currentClassName + "_" + currentMethodName + ":"
    );

    node->visit_children(this);

    gen(
    	" # End Method Node: " + currentMethodName
    );
}

void CodeGenerator::visitMethodBodyNode(MethodBodyNode* node) {
    gen(
    	" # Begin Method Body Node",
    	"push %ebp",
    	"mov %esp, %ebp",
    	"sub $" + std::to_string(currentMethodInfo.localsSize) + ", %esp",
    	"push %ebx",
    	"push %esi",
    	"push %edi"
    );


    node->visit_children(this);

    if (currentMethodName == currentClassName) {
    	gen(
    		"mov 8(%ebp), %eax"
    	);
    }

    gen(
    	"pop %edi",
    	"pop %esi",
    	"pop %ebx",
    	"mov %ebp, %esp",
    	"pop %ebp",
    	"ret",
    	" # End Method Body Node"
    );


}

void CodeGenerator::visitParameterNode(ParameterNode* node) {
    node->visit_children(this);
}

void CodeGenerator::visitDeclarationNode(DeclarationNode* node) {
    node->visit_children(this);
}

void CodeGenerator::visitReturnStatementNode(ReturnStatementNode* node) {
	node->visit_children(this);

	gen(
		" # Return Statement Node",
		"pop %eax"
	);
}

void CodeGenerator::visitAssignmentNode(AssignmentNode* node) {
    node->visit_children(this);

    std::cout << " # Begin Assignment Node: ";
    if (!node->identifier_2) {
    	std::cout << node->identifier_1->name << std::endl;
    } else {
    	std::cout << node->identifier_1->name + "(" + node->identifier_1->objectClassName + ")." + node->identifier_2->name << std::endl;
    }

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
		"mov %eax, " + std::to_string(offset) + "(%ebp)",
		" # End Assignment Node"
	);
}

void CodeGenerator::visitCallNode(CallNode* node) {
    node->visit_children(this);
}

void CodeGenerator::visitIfElseNode(IfElseNode* node) {
    node->expression->accept(this);

    std::string currentLabel = std::to_string(nextLabel());

    gen(
    	" # Begin If Else Node"
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
		"end" + currentLabel + ":",
		" # End If Else Node"
	);
}

void CodeGenerator::visitWhileNode(WhileNode* node) {
    std::string currentLabel = std::to_string(nextLabel());
	gen(
		" # Begin While Node",
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
		"loopend" + currentLabel + ":",
		" # End While Node"
	);
}

void CodeGenerator::visitPrintNode(PrintNode* node) {
   node->visit_children(this);

   gen(
   		" # Begin Print Node",
   		"push $printstr",
   		"call printf",
   		"add $8, %esp",
   		" # End Print Node"
   );
}

void CodeGenerator::visitDoWhileNode(DoWhileNode* node) {
	std::string currentLabel = std::to_string(nextLabel());
	gen(
		" # Begin Do While Node",
		"loopstart" + currentLabel + ":"
 	);

	node->visit_children(this);

	gen(
		"pop %eax",
		"cmp $1, %eax",
		"je loopstart" + currentLabel,
		" # End Do While Node"		
	);
}

void CodeGenerator::visitPlusNode(PlusNode* node) {
    node->visit_children(this);

	gen(
		" # Begin Plus Node",
		"pop %eax",
		"pop %ebx",
		"add %ebx, %eax",
		"push %eax",
		" # End Plus Node"
	);
}

void CodeGenerator::visitMinusNode(MinusNode* node) {
	node->visit_children(this);

	gen(
		" # Begin Minus Node",
		"pop %ebx",
		"pop %eax",
		"sub %ebx, %eax",
		"push %eax",
		" # End Minus Node"
	);
}

void CodeGenerator::visitTimesNode(TimesNode* node) {
	node->visit_children(this);

	gen(
		" # Begin Times Node",
		"pop %eax",
		"pop %ebx",
		"imul %ebx, %eax",
		"push %eax",
		" # End Times Node"
	);
}

void CodeGenerator::visitDivideNode(DivideNode* node) {
	node->visit_children(this);

	gen(
		" # Begin Divide Node",
		"pop %ebx",
		"pop %eax",
		"cdq",
		"idiv %ebx",
		"push %eax",
		" # End Divide Node"
	);
}

void CodeGenerator::visitGreaterNode(GreaterNode* node) {
	node->visit_children(this);

	std::string currentLabel = std::to_string(nextLabel());
	gen(
		" # Begin Greater Node",
		"pop %ebx",
		"pop %eax",
		"cmp %ebx, %eax",
		"jg greater" + currentLabel,
		"mov $0, %eax",
		"jmp done" + currentLabel,
		"greater" + currentLabel + ":",
		"mov $1, %eax",
		"done" + currentLabel + ":",
		"push %eax",
		" # End Greater Node"
	);
}

void CodeGenerator::visitGreaterEqualNode(GreaterEqualNode* node) {
	node->visit_children(this);

	std::string currentLabel = std::to_string(nextLabel());
	gen(
		" # Begin Greater Equal Node",
		"pop %ebx",
		"pop %eax",
		"cmp %ebx, %eax",
		"jge greaterequal" + currentLabel,
		"mov $0, %eax",
		"jmp done" + currentLabel,
		"greaterequal" + currentLabel + ":",
		"mov $1, %eax",
		"done" + currentLabel + ":",
		"push %eax",
		" # End Greater Equal Node"
	);
}

void CodeGenerator::visitEqualNode(EqualNode* node) {
	node->visit_children(this);

	std::string currentLabel = std::to_string(nextLabel());
	gen(
		" # Begin Equal Node",
		"pop %eax",
		"pop %ebx",
		"cmp %ebx, %eax",
		"je equal" + currentLabel,
		"mov $0, %eax",
		"jmp done" + currentLabel,
		"equal" + currentLabel + ":",
		"mov $1, %eax",
		"done" + currentLabel + ":",
		"push %eax",
		" # End Equal Node"
	);
}

void CodeGenerator::visitAndNode(AndNode* node) {
	node->visit_children(this);

	gen(
		" # Begin And Node",
		"pop %eax",
		"pop %ebx",
		"and %ebx, %eax",
		"push %eax",
		" # End And Node"
	);
}

void CodeGenerator::visitOrNode(OrNode* node) {
	node->visit_children(this);

	gen(
		" # Begin Or Node",
		"pop %eax",
		"pop %ebx",
		"or %ebx, %eax",
		"push %eax",
		" # End Or Node"
	);
}

void CodeGenerator::visitNotNode(NotNode* node) {
	node->visit_children(this);

	gen(
		" # Begin Not Node",
		"mov $1, %ebx",
		"pop %eax",
		"xor %ebx, %eax",
		"push %eax",
		" # End Not Node"
	);
}

void CodeGenerator::visitNegationNode(NegationNode* node) {
	node->visit_children(this);

	gen(
		" # Begin Negation Node",
		"pop %eax",
		"neg %eax",
		"push %eax",
		" # End Negation Node"
	);
}

void CodeGenerator::visitMethodCallNode(MethodCallNode* node) {
    std::cout << " # Begin Method Call Node: ";
    if (!node->identifier_2) {
    	std::cout << node->identifier_1->name << std::endl;
    } else {
    	std::cout << node->identifier_1->name + "(" + node->identifier_1->objectClassName + ")." + node->identifier_2->name << std::endl;
    }


    gen(
    	"push %eax",
    	"push %ecx",
    	"push %edx"
    );

    int numArgs = 0;
    if (node->expression_list) {
   		numArgs = node->expression_list->size();
   		for (std::list<ExpressionNode*>::reverse_iterator iter = node->expression_list->rbegin(); iter != node->expression_list->rend(); iter++) {
   			(*iter)->accept(this);
   		}
	}

	int offset = 0;
	std::string className = "";
	std::string methodName = "";

	if (!node->identifier_2) {
		methodName = node->identifier_1->name;
		className = currentClassName;

		offset += 8;
	} else {
		methodName = node->identifier_2->name;
		className = node->identifier_1->objectClassName;

		if (currentMethodInfo.variables->count(node->identifier_1->name)) {
			offset += currentMethodInfo.variables->at(node->identifier_1->name).offset;
		} 
		else {
			offset += 8 + currentClassInfo.members->at(node->identifier_1->name).offset;
		}
	}
	while(!classTable->at(className).methods->count(methodName)) {
			className = classTable->at(className).superClassName;
	}

	gen(
		"push " + std::to_string(offset) + "(%ebp)",
		"call " + className + "_" + methodName,
		"add $" + std::to_string(4 * (numArgs + 1)) + ", %esp",
		"mov %eax, %edi",
		"pop %edx",
    	"pop %ecx",
    	"pop %eax",
    	"mov %edi, %eax",
    	" # End Method Call Node"
	);
}

void CodeGenerator::visitMemberAccessNode(MemberAccessNode* node) {
    node->visit_children(this);

    gen(" # Begin Member Access Node: " + node->identifier_1->name + "(" + node->identifier_1->objectClassName + ")." + node->identifier_2->name);

    int variableOffset = classTable->at(node->identifier_1->objectClassName).members->at(node->identifier_2->name).offset;
    int memberOffset = 0;
    if (currentMethodInfo.variables->count(node->identifier_1->name)) {
    	memberOffset += currentMethodInfo.variables->at(node->identifier_1->name).offset;
    } else {
    	memberOffset += 8 + currentClassInfo.members->at(node->identifier_1->name).offset;
    }

    gen(
    	"mov " + std::to_string(memberOffset) + "(%ebp), %eax",
    	"push " + std::to_string(variableOffset) + "(%eax)",
    	" # End Member Access"
    );
}

void CodeGenerator::visitVariableNode(VariableNode* node) {
    node->visit_children(this);

    gen(" # Begin Variable Node: " + node->identifier->name);
    
    if (currentMethodInfo.variables->count(node->identifier->name)) {
    	// local variable
    	gen(
    		"push " + std::to_string(currentMethodInfo.variables->
    			at(node->identifier->name).offset) + "(%ebp)"
    	);
    } else {
    	// member variable
    	gen(
    		"push " + std::to_string(8 + currentClassInfo.members->
    			at(node->identifier->name).offset) + "(%ebp)"
    	);
    	//NEED TO SEPARATE MEMBER ACCESS THEN VARIABLE ACCESS
    }

    gen(" # End Variable Node");
}

void CodeGenerator::visitIntegerLiteralNode(IntegerLiteralNode* node) {
    node->visit_children(this);

    gen(" # Begin Integer Literal Node",
    	"push $" + std::to_string(node->integer->value),
    	" # End Integer Literal Node"
    );
}

void CodeGenerator::visitBooleanLiteralNode(BooleanLiteralNode* node) {
    node->visit_children(this);

    gen(" # Begin Boolean Literal Node",
    	"push $" + std::to_string(node->integer->value),
    	" # End Boolean Literal Node"
    );
}

void CodeGenerator::visitNewNode(NewNode* node) {
    //node->visit_children(this);

    std::string objectSize = std::to_string(classTable->at(node->identifier->name).membersSize);

    gen(
    	" # Begin New Node: " + node->identifier->name,
    	"push $" + objectSize,
    	"call malloc",
   		"add $4, %esp",
   		"mov %eax, %edi"
   	);

    if (classTable->at(node->identifier->name).methods->count(node->identifier->name)) {
    	gen(
    		"push %eax",
    		"push %ecx",
    		"push %edx"
    	);

    	int numArgs = 0;
    	if (node->expression_list) {
    		numArgs = node->expression_list->size();
   			for (std::list<ExpressionNode*>::reverse_iterator iter = node->expression_list->rbegin(); iter != node->expression_list->rend(); iter++) {
   				(*iter)->accept(this);
   			}
		}

		gen(
			"push %edi",
			"call " + node->identifier->name + "_" + node->identifier->name,
			"add $" + std::to_string(4 * (numArgs + 1)) + ", %esp",
			"mov %eax, %edi",
			"pop %edx",
    		"pop %ecx",
    		"pop %eax",
    		"mov %edi, %eax"
		);
    }
    gen(
    	"push %eax",
    	" # End New Node"
   	);
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
