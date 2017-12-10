#include "typecheck.hpp"

// Defines the function used to throw type errors. The possible
// type errors are defined as an enumeration in the header file.
void typeError(TypeErrorCode code) {
  switch (code) {
    case undefined_variable:
      std::cerr << "Undefined variable." << std::endl;
      break;
    case undefined_method:
      std::cerr << "Method does not exist." << std::endl;
      break;
    case undefined_class:
      std::cerr << "Class does not exist." << std::endl;
      break;
    case undefined_member:
      std::cerr << "Class member does not exist." << std::endl;
      break;
    case not_object:
      std::cerr << "Variable is not an object." << std::endl;
      break;
    case expression_type_mismatch:
      std::cerr << "Expression types do not match." << std::endl;
      break;
    case argument_number_mismatch:
      std::cerr << "Method called with incorrect number of arguments." << std::endl;
      break;
    case argument_type_mismatch:
      std::cerr << "Method called with argument of incorrect type." << std::endl;
      break;
    case while_predicate_type_mismatch:
      std::cerr << "Predicate of while loop is not boolean." << std::endl;
      break;
    case do_while_predicate_type_mismatch:
      std::cerr << "Predicate of do while loop is not boolean." << std::endl;
      break;
    case if_predicate_type_mismatch:
      std::cerr << "Predicate of if statement is not boolean." << std::endl;
      break;
    case assignment_type_mismatch:
      std::cerr << "Left and right hand sides of assignment types mismatch." << std::endl;
      break;
    case return_type_mismatch:
      std::cerr << "Return statement type does not match declared return type." << std::endl;
      break;
    case constructor_returns_type:
      std::cerr << "Class constructor returns a value." << std::endl;
      break;
    case no_main_class:
      std::cerr << "The \"Main\" class was not found." << std::endl;
      break;
    case main_class_members_present:
      std::cerr << "The \"Main\" class has members." << std::endl;
      break;
    case no_main_method:
      std::cerr << "The \"Main\" class does not have a \"main\" method." << std::endl;
      break;
    case main_method_incorrect_signature:
      std::cerr << "The \"main\" method of the \"Main\" class has an incorrect signature." << std::endl;
      break;
  }
  exit(1);
}

static CompoundType typeMap(TypeNode* t) {
  CompoundType* c = new CompoundType();
  c->objectClassName = "";
  if (dynamic_cast<const IntegerTypeNode*>(t) != NULL) {
    c->baseType = bt_integer;
  } else if (dynamic_cast<const BooleanTypeNode*>(t) != NULL) {
    c->baseType = bt_boolean;
  } else if (dynamic_cast<const NoneNode*>(t) != NULL) {
    c->baseType = bt_none;
  } else if (dynamic_cast<const ObjectTypeNode*>(t) != NULL) {
    c->baseType = bt_object;
    c->objectClassName = ((ObjectTypeNode*)t)->identifier->name;
  }
  return *c;
}

// TypeCheck Visitor Functions: These are the functions you will
// complete to build the symbol table and type check the program.
// Not all functions must have code, many may be left empty.

void TypeCheck::visitProgramNode(ProgramNode* node) {
  classTable = new ClassTable();

  node->visit_children(this);

  if (!classTable->count("Main")) {
      typeError(no_main_class);
  }
}

void TypeCheck::visitClassNode(ClassNode* node) {
  ClassInfo* classInfo = new ClassInfo();

  std::string name = node->identifier_1->name;
  currentClassName = name;

  if (node->identifier_2) {
    classInfo->superClassName = node->identifier_2->name;
    if ((*classTable).count(classInfo->superClassName) == 0) {
      typeError(undefined_class);
    }
  } else {
    classInfo->superClassName = "";
  }

  classInfo->methods = new MethodTable();

  classInfo->members = new VariableTable();

  classTable->insert(std::pair<std::string, ClassInfo>(name, *classInfo));
  delete classInfo;

  if (!node->identifier_1->name.compare("Main") && node->declaration_list->size() > 0) {
    typeError(main_class_members_present);
  }

  // visit local members
  currentVariableTable = (*classTable)[name].members;
  currentMemberOffset = 0;
  std::list<DeclarationNode*>* d = node->declaration_list;
  if (d) {
    for (std::list<DeclarationNode*>::iterator it = d->begin(); it != d->end(); it++) {
      VariableInfo* v = new VariableInfo();
      v->type = typeMap((*it)->type);
      v->offset = currentMemberOffset;
      currentMemberOffset += 4;
      v->size = 4;

      std::string memberName = (*it)->identifier_list->front()->name;
      currentVariableTable->insert(std::pair<std::string, VariableInfo>(memberName, *v));

      delete v;
    }
  }

  // visit super class members 
  if (node->identifier_2) {
    std::string currentClassName = node->identifier_2->name;
    while (currentClassName.compare("")) {
      ClassInfo currentInfo = classTable->at(currentClassName);
      for (std::map<std::string, VariableInfo>::iterator it = currentInfo.members->begin(); it != currentInfo.members->end(); it++) {
        VariableInfo* v = new VariableInfo();
        v->type = it->second.type;
        v->offset = currentMemberOffset;
        currentMemberOffset += 4;
        v->size = 4;

        std::string memberName = it->first;
        currentVariableTable->insert(std::pair<std::string, VariableInfo>(memberName, *v));

        delete v;
      }

      currentClassName = currentInfo.superClassName;
    }
  }

  (*classTable)[name].membersSize = currentMemberOffset;



  // visit methods
  currentVariableTable = NULL;
  std::list<MethodNode*>* m = node->method_list;
  if (m) {
    for (std::list<MethodNode*>::iterator it = m->begin(); it != m->end(); it++) {
      visitMethodNode(*it);
    }
  }

  if (!node->identifier_1->name.compare("Main")) {
    if (!(*classTable)[node->identifier_1->name].methods->count("main")) {
      typeError(no_main_method);
    } else if ((*classTable)[node->identifier_1->name].methods->at("main").parameters->size() > 0) {
      typeError(main_method_incorrect_signature);
    }
  }
}

void TypeCheck::visitMethodNode(MethodNode* node) {
  

  MethodInfo* methodInfo = new MethodInfo();
  currentMethodTable = (*classTable)[currentClassName].methods;

  std::string name = node->identifier->name;

  CompoundType x = typeMap(node->type);
  methodInfo->returnType = x;
  node->methodbody->basetype = x.baseType;
  node->methodbody->objectClassName = x.objectClassName;

  methodInfo->variables = new VariableTable();
  currentLocalOffset = -4;
  currentVariableTable = methodInfo->variables;
  // std::cout << "curVarTable updated" << std::endl;

  

  // need to check that parameters are of correct type
  methodInfo->parameters = new std::list<CompoundType>();
  currentParameterOffset = 12;
  std::list<ParameterNode*>* p = node->parameter_list;

  node->visit_children(this);

  if (p) {
    for (std::list<ParameterNode*>::iterator it = p->begin(); it != p->end(); it++) {
      methodInfo->parameters->push_back(typeMap((*it)->type));
    }
  }

  currentMethodTable->insert(std::pair<std::string, MethodInfo>(name, *methodInfo));
  delete methodInfo;

  if (!node->identifier->name.compare(currentClassName)) {
    if (node->type->basetype != bt_none) {
      typeError(constructor_returns_type);
    }
  }

  (*currentMethodTable)[name].localsSize = (-1) * (currentLocalOffset + 4);// + (currentParameterOffset - 12); 
}

void TypeCheck::visitMethodBodyNode(MethodBodyNode* node) {
  node->visit_children(this);

  if (node->returnstatement != NULL) {
    if (node->basetype != node->returnstatement->basetype || node->objectClassName != node->returnstatement->objectClassName) {
      typeError(return_type_mismatch);
    }
  } else {
    if (node->basetype != bt_none) {
      typeError(return_type_mismatch);
    }
  }
}

void TypeCheck::visitParameterNode(ParameterNode* node) {
  CompoundType t = typeMap(node->type);
  node->basetype = t.baseType;
  node->objectClassName = t.objectClassName;

  VariableInfo* v = new VariableInfo();
  v->type = t;
  v->offset = currentParameterOffset;
  currentParameterOffset += 4;
  v->size = 4;
  currentVariableTable->insert(std::pair<std::string, VariableInfo>(node->identifier->name, *v));

  delete v;
}

void TypeCheck::visitDeclarationNode(DeclarationNode* node) {
  CompoundType t = typeMap(node->type);
  node->basetype = t.baseType;
  node->objectClassName = t.objectClassName;
  std::list<IdentifierNode*>* i = node->identifier_list;
  for (std::list<IdentifierNode*>::iterator it = i->begin(); it != i->end(); it++) {
    (*it)->basetype = t.baseType;
    (*it)->objectClassName = t.objectClassName;
    VariableInfo* v = new VariableInfo();
    v->type = t;
    v->offset = currentLocalOffset;
    currentLocalOffset -= 4;
    v->size = 4;
    if (currentVariableTable) {
      currentVariableTable->insert(std::pair<std::string, VariableInfo>((*it)->name, *v));
    }
    
    delete v;
  }
}

void TypeCheck::visitReturnStatementNode(ReturnStatementNode* node) {
  node->visit_children(this);
  node->basetype = node->expression->basetype;
  node->objectClassName = node->expression->objectClassName;
}

void TypeCheck::visitAssignmentNode(AssignmentNode* node) {
  node->visit_children(this);

  bool found = false;
  bool dotOp = false;

  if (node->identifier_2 != NULL) {
    dotOp = true;

    if (currentVariableTable->count(node->identifier_1->name)) {
      found = true;
      node->identifier_1->basetype = (*currentVariableTable)[node->identifier_1->name].type.baseType;
      node->identifier_1->objectClassName = (*currentVariableTable)[node->identifier_1->name].type.objectClassName;
    }
    std::string className = currentClassName;
    if (!found) {
      while(className.compare("")) {
        if ((*classTable)[className].members->count(node->identifier_1->name)) {
          found = true;
          node->identifier_1->basetype = (*classTable)[className].members->at(node->identifier_1->name).type.baseType;
          node->identifier_1->objectClassName = (*classTable)[className].members->at(node->identifier_1->name).type.objectClassName;
          break;
        }
        className = (*classTable)[className].superClassName;
      }
    }

    // found
    if (found) {
      // if id 1 is not a class, typeerror not_object
      if (!classTable->count(node->identifier_1->objectClassName)) {
        typeError(not_object);
      }
      // check id 2 is a member of id 1
      VariableTable* memberTable = (*classTable)[node->identifier_1->objectClassName].members;
      if (memberTable->count(node->identifier_2->name)) {
        node->identifier_2->basetype = memberTable->at(node->identifier_2->name).type.baseType;
        node->identifier_2->objectClassName = memberTable->at(node->identifier_2->name).type.objectClassName;
        if (node->identifier_2->basetype != node->expression->basetype || node->identifier_2->objectClassName != node->expression->objectClassName) {
          std::cout << "1: " << node->identifier_2->basetype << "," << node->expression->basetype << std::endl;
          typeError(assignment_type_mismatch);
        }
      } else {
        typeError(undefined_member);
      }
    } else { // variable not found
      typeError(undefined_variable);
    }
  } else { // no dot op
    // check id 1 locally
    if (currentVariableTable->count(node->identifier_1->name)) {
      found = true;
    }
    // check id 1 in members
    if (!found) {
      std::string className = currentClassName;
      while(className.compare("")) {
        if ((*classTable)[className].members->count(node->identifier_1->name)) {
          found = true;
          break;
        }
        className = (*classTable)[className].superClassName;
      }
    }

    if (found) {
      CompoundType idC = (*currentVariableTable)[node->identifier_1->name].type;
      if (idC.baseType != node->expression->basetype || idC.objectClassName != node->expression->objectClassName) {
          typeError(assignment_type_mismatch);
      }

    } else {
      typeError(undefined_variable);
    }
  }
}

void TypeCheck::visitCallNode(CallNode* node) {
  node->visit_children(this);
}

void TypeCheck::visitIfElseNode(IfElseNode* node) {
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean) {
    typeError(if_predicate_type_mismatch);
  }
}

void TypeCheck::visitWhileNode(WhileNode* node) {
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean) {
    typeError(while_predicate_type_mismatch);
  }
}

void TypeCheck::visitDoWhileNode(DoWhileNode* node) {
  node->visit_children(this);
  if (node->expression->basetype != bt_boolean) {
    typeError(do_while_predicate_type_mismatch);
  }
}

void TypeCheck::visitPrintNode(PrintNode* node) {
  node->visit_children(this);
}

void TypeCheck::visitPlusNode(PlusNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) {
    node->basetype = bt_integer;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitMinusNode(MinusNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) {
    node->basetype = bt_integer;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitTimesNode(TimesNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) {
    node->basetype = bt_integer;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitDivideNode(DivideNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) {
    node->basetype = bt_integer;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitGreaterNode(GreaterNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) {
    node->basetype = bt_boolean;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitGreaterEqualNode(GreaterEqualNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer) {
    node->basetype = bt_boolean;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitEqualNode(EqualNode* node) {
  node->visit_children(this);

  if ((node->expression_1->basetype == bt_boolean && node->expression_2->basetype == bt_boolean) || (node->expression_1->basetype == bt_integer && node->expression_2->basetype == bt_integer)) {
    node->basetype = bt_boolean;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitAndNode(AndNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_boolean && node->expression_2->basetype == bt_boolean) {
    node->basetype = bt_boolean;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitOrNode(OrNode* node) {
  node->visit_children(this);

  if (node->expression_1->basetype == bt_boolean && node->expression_2->basetype == bt_boolean) {
    node->basetype = bt_boolean;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitNotNode(NotNode* node) {
  node->visit_children(this);

  if (node->expression->basetype == bt_boolean) {
    node->basetype = bt_boolean;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void TypeCheck::visitNegationNode(NegationNode* node) {
  node->visit_children(this);

  if (node->expression->basetype == bt_integer) {
    node->basetype = bt_integer;
    node->objectClassName = "";
  } else {
    typeError(expression_type_mismatch);
  }
}

void checkArguments(std::string methodName, MethodTable* methodTable, std::list<ExpressionNode*>* callParamList) {
  int numParams = (callParamList == NULL) ? 0 : callParamList->size();
  std::list<CompoundType>* p = methodTable->at(methodName).parameters;
  int expectedNumParams = !(methodTable->count(methodName)) ? 0 : p->size();
  if (numParams != expectedNumParams) {
    typeError(argument_number_mismatch);
  }
  if (expectedNumParams != 0) {
    std::list<CompoundType>::iterator expectedParamIt = p->begin();
    for (std::list<ExpressionNode*>::iterator callParamIt = callParamList->begin(); callParamIt != callParamList->end(); callParamIt++) {
      CompoundType* c = new CompoundType();
      c->baseType = (*callParamIt)->basetype;
      c->objectClassName = (*callParamIt)->objectClassName;
      if (c->baseType != expectedParamIt->baseType || c->objectClassName != expectedParamIt->objectClassName) {
        
        typeError(argument_type_mismatch);
      }
      expectedParamIt++;
    }
  }

  
}

void TypeCheck::visitMethodCallNode(MethodCallNode* node) {
  node->visit_children(this);

  bool found = false;
  if (node->identifier_2 == NULL) {
    std::string myMethod = node->identifier_1->name;
    std::string className = currentClassName;
    while(className.compare("")) {
      if ((*classTable)[className].methods->count(myMethod)) {
        found = true;
        checkArguments(myMethod, (*classTable)[className].methods, node->expression_list);
        node->identifier_1->basetype = (*classTable)[className].methods->at(myMethod).returnType.baseType;
        node->identifier_1->objectClassName = (*classTable)[className].methods->at(myMethod).returnType.objectClassName;
        break;
        
      }
      className = (*classTable)[className].superClassName;
    }
    if (!found) {
      typeError(undefined_method);
    }
  } else {
    if (currentVariableTable->count(node->identifier_1->name)) {
      found = true;
      node->identifier_1->basetype = (*currentVariableTable)[node->identifier_1->name].type.baseType;
      node->identifier_1->objectClassName = (*currentVariableTable)[node->identifier_1->name].type.objectClassName;
    }
    std::string className = currentClassName;
    if (!found) {
      while(className.compare("")) {
        if ((*classTable)[className].members->count(node->identifier_1->name)) {
          found = true;
          node->identifier_1->basetype = (*classTable)[className].members->at(node->identifier_1->name).type.baseType;
          node->identifier_1->objectClassName = (*classTable)[className].members->at(node->identifier_1->name).type.objectClassName;
          break;
        }
        className = (*classTable)[className].superClassName;
      }
    }

    if (found) {
      found = false;
      std::string myMethod = node->identifier_2->name;
      if (!classTable->count(node->identifier_1->objectClassName)) {
        typeError(not_object);
      }

      std::string className = node->identifier_1->objectClassName;
      while(className.compare("")) {
        if ((*classTable)[className].methods->count(myMethod)) {
          found = true;
          checkArguments(myMethod, (*classTable)[className].methods, node->expression_list);
          node->basetype = (*classTable)[className].methods->at(myMethod).returnType.baseType;
          node->objectClassName = (*classTable)[className].methods->at(myMethod).returnType.objectClassName;
          break;
        }
        className = (*classTable)[className].superClassName;
      }
      if (!found) {
        typeError(undefined_method);
      }
    } else {
      typeError(undefined_variable);
    }
  }
}

void TypeCheck::visitMemberAccessNode(MemberAccessNode* node) {
  node->visit_children(this);

  bool found = false;
  if (currentVariableTable->count(node->identifier_1->name)) {
    found = true;
    node->identifier_1->basetype = (*currentVariableTable)[node->identifier_1->name].type.baseType;
    node->identifier_1->objectClassName = (*currentVariableTable)[node->identifier_1->name].type.objectClassName;
  }
  std::string className = currentClassName;
  if (!found) {
    while(className.compare("")) {
      if ((*classTable)[className].members->count(node->identifier_1->name)) {
        found = true;
        node->identifier_1->basetype = (*classTable)[className].members->at(node->identifier_1->name).type.baseType;
        node->identifier_1->objectClassName = (*classTable)[className].members->at(node->identifier_1->name).type.objectClassName;
        break;
      }
      className = (*classTable)[className].superClassName;
    }
  }

  // found
  if (found) {
    if (!classTable->count(node->identifier_1->objectClassName)) {
      typeError(not_object);
    }
    if ((*classTable)[node->identifier_1->objectClassName].members->count(node->identifier_2->name)) {
      node->identifier_2->basetype = (*classTable)[node->identifier_1->objectClassName].members->at(node->identifier_2->name).type.baseType;
      node->identifier_2->objectClassName = (*classTable)[node->identifier_1->objectClassName].members->at(node->identifier_2->name).type.objectClassName;
    }

    bool id2Found = false;
    std::string className = node->identifier_1->objectClassName;
    while(className.compare("")) {
      if ((*classTable)[className].members->count(node->identifier_2->name)) {
        id2Found = true;
        node->identifier_2->basetype = (*classTable)[className].members->at(node->identifier_2->name).type.baseType;
        node->identifier_2->objectClassName = (*classTable)[className].members->at(node->identifier_2->name).type.objectClassName;
        break;
      }
      className = (*classTable)[className].superClassName;
    }
    if (!id2Found) {
      typeError(undefined_member);
    }

    node->basetype = node->identifier_2->basetype;
    node->objectClassName = node->identifier_2->objectClassName;
  } else { // variable not found
    typeError(undefined_variable);
  }  
}

void TypeCheck::visitVariableNode(VariableNode* node) {
  bool found = false;

  std::string className = currentClassName;
  VariableTable* variableTable = currentVariableTable;
  IdentifierNode* identifier = node->identifier;

  // Check local variables
  if ((*variableTable).count(identifier->name)) {
    found = true;
    CompoundType c = (*variableTable)[identifier->name].type;
    node->basetype = c.baseType;
    node->objectClassName = "";
    if (node->basetype == bt_object) {
      node->objectClassName = c.objectClassName;
    }
  }
  // Check member variables
  else if ((*classTable)[className].members->count(identifier->name)) {
    found = true;
    CompoundType c = (*classTable)[className].members->at(identifier->name).type;
    node->basetype = c.baseType;
    node->objectClassName = "";
    if (node->basetype == bt_object) {
      node->objectClassName = c.objectClassName;
    }
  } 

  if (!found) {
    // Check all superclass variables
    std::string superName = (*classTable)[className].superClassName;
    while(superName.compare("")) {
      if ((*classTable)[superName].members->count(identifier->name)) {
        found = true;
        CompoundType c = (*classTable)[superName].members->at(identifier->name).type;
        node->basetype = c.baseType;
        node->objectClassName = "";
        if (node->basetype == bt_object) {
          node->objectClassName = c.objectClassName;
        }
        break;
      }
      superName = (*classTable)[superName].superClassName;
    }
  }

  if(!found) {
    typeError(undefined_variable);
  }
}

void TypeCheck::visitIntegerLiteralNode(IntegerLiteralNode* node) {
  node->basetype = bt_integer;
  node->objectClassName = "";
}

void TypeCheck::visitBooleanLiteralNode(BooleanLiteralNode* node) {
  node->basetype = bt_boolean;
  node->objectClassName = "";
}

void TypeCheck::visitNewNode(NewNode* node) {
  if ((*classTable).count(node->identifier->name)) {
    node->basetype = bt_object;
    node->objectClassName = node->identifier->name;
  } else {
    typeError(undefined_class);
  }
}

void TypeCheck::visitIntegerTypeNode(IntegerTypeNode* node) {
  node->basetype = bt_integer;
  node->objectClassName = "";
}

void TypeCheck::visitBooleanTypeNode(BooleanTypeNode* node) {
  node->basetype = bt_boolean;
  node->objectClassName = "";
}

void TypeCheck::visitObjectTypeNode(ObjectTypeNode* node) {
  node->basetype = bt_object;
  node->objectClassName = node->identifier->name;
}

void TypeCheck::visitNoneNode(NoneNode* node) {
  node->basetype = bt_none;
  node->objectClassName = "";
}

void TypeCheck::visitIdentifierNode(IdentifierNode* node) {
  // WRITEME: Replace with code if necessary
}

void TypeCheck::visitIntegerNode(IntegerNode* node) {
  node->basetype = bt_integer;
  node->objectClassName = "";
}



// The following functions are used to print the Symbol Table.
// They do not need to be modified at all.

std::string genIndent(int indent) {
  std::string string = std::string("");
  for (int i = 0; i < indent; i++)
    string += std::string(" ");
  return string;
}

std::string string(CompoundType type) {
  switch (type.baseType) {
    case bt_integer:
      return std::string("Integer");
    case bt_boolean:
      return std::string("Boolean");
    case bt_none:
      return std::string("None");
    case bt_object:
      return std::string("Object(") + type.objectClassName + std::string(")");
    default:
      return std::string("");
  }
}


void print(VariableTable variableTable, int indent) {
  std::cout << genIndent(indent) << "VariableTable {";
  if (variableTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (VariableTable::iterator it = variableTable.begin(); it != variableTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << string(it->second.type);
    std::cout << ", " << it->second.offset << ", " << it->second.size << "}";
    if (it != --variableTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(MethodTable methodTable, int indent) {
  std::cout << genIndent(indent) << "MethodTable {";
  if (methodTable.size() == 0) {
    std::cout << "}";
    return;
  }
  std::cout << std::endl;
  for (MethodTable::iterator it = methodTable.begin(); it != methodTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    std::cout << genIndent(indent + 4) << string(it->second.returnType) << "," << std::endl;
    std::cout << genIndent(indent + 4) << it->second.localsSize << "," << std::endl;
    print(*it->second.variables, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --methodTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}";
}

void print(ClassTable classTable, int indent) {
  std::cout << genIndent(indent) << "ClassTable {" << std::endl;
  for (ClassTable::iterator it = classTable.begin(); it != classTable.end(); it++) {
    std::cout << genIndent(indent + 2) << it->first << " -> {" << std::endl;
    if (it->second.superClassName != "")
      std::cout << genIndent(indent + 4) << it->second.superClassName << "," << std::endl;
    print(*it->second.members, indent + 4);
    std::cout << "," << std::endl;
    print(*it->second.methods, indent + 4);
    std::cout <<std::endl;
    std::cout << genIndent(indent + 2) << "}";
    if (it != --classTable.end())
      std::cout << ",";
    std::cout << std::endl;
  }
  std::cout << genIndent(indent) << "}" << std::endl;
}

void print(ClassTable classTable) {
  print(classTable, 0);
}
