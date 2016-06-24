// $Id: type_checker.cpp,v 1.40 2016/05/19 18:57:22 ist177983 Exp $ -*- c++ -*-
#include <string>
#include "targets/type_checker.h"
#include "ast/all.h"  // automatically generated
#include <vector>
#include <algorithm>
#define ASSERT_UNSPEC \
    { if (node->type() != nullptr && \
          node->type()->name() != basic_type::TYPE_UNSPEC) return; }

//---------------------------------------------------------------------------
size_t offset = 0;
size_t argOffset = 0;

void zu::type_checker::do_integer_node(cdk::integer_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(4, basic_type::TYPE_INT));
}

void zu::type_checker::do_double_node(cdk::double_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
}

void zu::type_checker::do_string_node(cdk::string_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(4, basic_type::TYPE_STRING));
}

//---------------------------------------------------------------------------

inline void zu::type_checker::processUnaryExpression(cdk::unary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->type()->name() == basic_type::TYPE_UNSPEC)
    node->argument()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->argument()->type()->name() == basic_type::TYPE_INT )
    node->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->argument()->type()->name() == basic_type::TYPE_DOUBLE)
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
  else
    throw std::string("wrong type in argument of unary expression");
}

void zu::type_checker::do_neg_node(cdk::neg_node * const node, int lvl) {
  processUnaryExpression(node, lvl);
}

//---------------------------------------------------------------------------


/* MULT | DIV | GE | GT | LE | LT*/
inline void zu::type_checker::processBinaryExpressionIntDouble(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  //case type unspec
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_INT )
    node->right()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_DOUBLE )
    node->right()->type(new basic_type(8, basic_type::TYPE_DOUBLE));

  if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_INT )
    node->left()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_DOUBLE )
    node->left()->type(new basic_type(8, basic_type::TYPE_DOUBLE));

  if (node->right()->type()->name() != basic_type::TYPE_INT && node->right()->type()->name() != basic_type::TYPE_DOUBLE){
    throw std::string("wrong type in right argument of binary expression");
  }

  if (node->left()->type()->name() != basic_type::TYPE_INT && node->left()->type()->name() != basic_type::TYPE_DOUBLE){
    throw std::string("wrong type in left argument of binary expression");
  }

//podem multiplicar-se e dividir se os tipos forem diferentes
  if (node->left()->type()->name() == node->right()->type()->name())
    node->type(node->right()->type());
  else 
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE));

}

/*ADD | EQ | NE*/
inline void zu::type_checker::processBinaryExpressionAdd(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;
  
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  //one is of type unspec
  if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_INT )
    node->right()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_DOUBLE )
    node->right()->type(new basic_type(8, basic_type::TYPE_DOUBLE));

  if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_INT )
    node->left()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_DOUBLE )
    node->left()->type(new basic_type(8, basic_type::TYPE_DOUBLE));


  if(node->left()->type()->name() == basic_type::TYPE_POINTER){
    if (node->right()->type()->name() != basic_type::TYPE_INT){
        throw std::string("wrong type in binary expression");
    }
    node->type(node->left()->type()); return;
  } else if(node->right()->type()->name() == basic_type::TYPE_POINTER){
    if (node->left()->type()->name() != basic_type::TYPE_INT){
        throw std::string("wrong type in binary expression");
    }
    node->type(node->right()->type()); return;
  }

  
  if (node->left()->type()->name() != basic_type::TYPE_INT && node->left()->type()->name() != basic_type::TYPE_DOUBLE ){
    throw std::string("wrong type in left argument of binary expression");
  }
  if (node->right()->type()->name() != basic_type::TYPE_INT && node->right()->type()->name() != basic_type::TYPE_DOUBLE ){
    throw std::string("wrong type in right argument of binary expression");
  }

  if (node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE)); 
  }
  else if (node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
  }
  else node->type(new basic_type(node->right()->type()->size(), node->right()->type()->name()));
}

/*SUB*/
inline void zu::type_checker::processBinaryExpressionSub(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;
  
  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);

  //one is of type unspec
  if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_INT )
    node->right()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_DOUBLE )
    node->right()->type(new basic_type(8, basic_type::TYPE_DOUBLE));

  if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_INT )
    node->left()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_DOUBLE )
    node->left()->type(new basic_type(8, basic_type::TYPE_DOUBLE));

  if (node->left()->type()->name() == basic_type::TYPE_UNSPEC && node->right()->type()->name() == basic_type::TYPE_POINTER )
    node->left()->type(new basic_type(4, basic_type::TYPE_INT));
  else if (node->right()->type()->name() == basic_type::TYPE_UNSPEC && node->left()->type()->name() == basic_type::TYPE_POINTER )
    node->right()->type(new basic_type(4, basic_type::TYPE_INT));


  if (node->left()->type()->name() != basic_type::TYPE_INT && node->left()->type()->name() != basic_type::TYPE_DOUBLE && node->left()->type()->name() != basic_type::TYPE_POINTER){
    throw std::string("wrong type in left argument of binary expression");
  }
  if (node->right()->type()->name() != basic_type::TYPE_INT && node->right()->type()->name() != basic_type::TYPE_DOUBLE && node->right()->type()->name() != basic_type::TYPE_POINTER){
    throw std::string("wrong type in right argument of binary expression");
  }


  if(node->left()->type()->name() == basic_type::TYPE_POINTER && node->right()->type()->name() == basic_type::TYPE_POINTER ){
    if(!checkPointer(node->left()->type(), node->right()->type())){
      throw std::string("pointers with different types in binary expression");
    } 
    node->type(new basic_type(4, basic_type::TYPE_INT));
    return;
  } else if ( node->left()->type()->name()== basic_type::TYPE_POINTER && node->right()->type()->name()!= basic_type::TYPE_INT ){
    throw std::string("types do not match in binary expression");
  } else if ( node->right()->type()->name()== basic_type::TYPE_POINTER && node->left()->type()->name()!= basic_type::TYPE_INT ){
    throw std::string("types do not match in binary expression");
  } 

  if ( node->left()->type()->name()== basic_type::TYPE_POINTER && node->right()->type()->name()== basic_type::TYPE_INT ){
    node->type(node->left()->type());
    return;
  } else if ( node->right()->type()->name()== basic_type::TYPE_POINTER && node->left()->type()->name()== basic_type::TYPE_INT ){
    throw "invalid operands";
  } 

  if (node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE)); 
  }
  else if (node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE));
  }
  
  node->type(new basic_type(node->right()->type()->size(), node->right()->type()->name()));
}

/*MOD | AND | OR | NOT*/
inline void zu::type_checker::processBinaryExpressionInt(cdk::binary_expression_node * const node, int lvl) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl + 2);
  node->right()->accept(this, lvl + 2);
  if (node->left()->type()->name() == basic_type::TYPE_UNSPEC)
    node->left()->type(new basic_type(4, basic_type::TYPE_INT));
  if (node->right()->type()->name() == basic_type::TYPE_UNSPEC)
    node->right()->type(new basic_type(4, basic_type::TYPE_INT));
  
  if (node->left()->type()->name() != basic_type::TYPE_INT)
    throw std::string("wrong type in left argument of binary expression");

  if (node->right()->type()->name() != basic_type::TYPE_INT)
    throw std::string("wrong type in right argument of binary expression");

  // in Zu, expressions are always int
  node->type(new basic_type(4, basic_type::TYPE_INT));
}


void zu::type_checker::do_add_node(cdk::add_node * const node, int lvl) {
  processBinaryExpressionAdd(node, lvl);
}
void zu::type_checker::do_sub_node(cdk::sub_node * const node, int lvl) {
  processBinaryExpressionSub(node, lvl);
}
void zu::type_checker::do_mul_node(cdk::mul_node * const node, int lvl) {
  processBinaryExpressionIntDouble(node, lvl);
}
void zu::type_checker::do_div_node(cdk::div_node * const node, int lvl) {
  processBinaryExpressionIntDouble(node, lvl);
}
void zu::type_checker::do_mod_node(cdk::mod_node * const node, int lvl) {
  processBinaryExpressionInt(node, lvl);
}
void zu::type_checker::do_lt_node(cdk::lt_node * const node, int lvl) {
  processBinaryExpressionIntDouble(node, lvl);
}
void zu::type_checker::do_le_node(cdk::le_node * const node, int lvl) {
  processBinaryExpressionIntDouble(node, lvl);
}
void zu::type_checker::do_ge_node(cdk::ge_node * const node, int lvl) {
  processBinaryExpressionIntDouble(node, lvl);
}
void zu::type_checker::do_gt_node(cdk::gt_node * const node, int lvl) {
  processBinaryExpressionIntDouble(node, lvl);
}
void zu::type_checker::do_ne_node(cdk::ne_node * const node, int lvl) {
  processBinaryExpressionAdd(node, lvl);
}
void zu::type_checker::do_eq_node(cdk::eq_node * const node, int lvl) {
  processBinaryExpressionAdd(node, lvl);
}

//---------------------------------------------------------------------------

void zu::type_checker::do_rvalue_node(zu::rvalue_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl);
  node->type(node->lvalue()->type());
}

//---------------------------------------------------------------------------

void zu::type_checker::do_assignment_node(zu::assignment_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lvalue()->accept(this, lvl+4);
  node->rvalue()->accept(this, lvl+4);
  std::string id;

  if (dynamic_cast<zu::identifier_node*>(node->lvalue())){
    id = *((dynamic_cast<zu::identifier_node*>(node->lvalue()))->name());
  }else{
    id = *(dynamic_cast<zu::identifier_node*>((dynamic_cast<zu::index_node*>(node->lvalue()))->pointer()))->name();
  }
  std::shared_ptr<symbol> sym = _symtab.find_local(id);

  if (!sym) sym = _symtab.find(id);
  if (!sym) throw id + " undeclared";

  if (node->lvalue()->type()->name() != node->rvalue()->type()->name() && (node->lvalue()->type()->name() == basic_type::TYPE_INT && node->rvalue()->type()->name() == basic_type::TYPE_DOUBLE))
    throw std::string("wrong types in assignment");
  else if (node->lvalue()->type()->name() == basic_type::TYPE_DOUBLE) 
    node->type(new basic_type(8, basic_type::TYPE_DOUBLE)); 
  else if(node->rvalue()->type()->name() == basic_type::TYPE_POINTER){
    node->type(node->lvalue()->type());
    node->rvalue()->type()->_subtype = node->lvalue()->type()->subtype();
    node->type()->_subtype = node->lvalue()->type()->subtype();
    if (node->lvalue()->type()->name() == basic_type::TYPE_POINTER){
      if (!checkPointer(new basic_type(*node->rvalue()->type()),new basic_type(*node->lvalue()->type()))){
        throw "wrong pointer operation";
      }
    }
  }else{ node->type(new basic_type(4, basic_type::TYPE_INT)); }
}

//---------------------------------------------------------------------------

void zu::type_checker::do_evaluation_node(zu::evaluation_node * const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void zu::type_checker::do_print_node(zu::print_node * const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
  if (node->argument()->type()->name() == basic_type::TYPE_UNSPEC){
    node->argument()->type(new basic_type(4, basic_type::TYPE_INT));
  }
  if (node->argument()->type()->name() == basic_type::TYPE_POINTER)
    throw "cannot print pointer";
}

//---------------------------------------------------------------------------

void zu::type_checker::do_read_node(zu::read_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(new basic_type(4, basic_type::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------
//New
void zu::type_checker::do_for_node(zu::for_node * const node, int lvl) {
  node->init()->accept(this, lvl + 4);
  node->condition()->accept(this, lvl + 4);
  node->incr()->accept(this, lvl + 4);
  node->block()->accept(this, lvl + 4);
}

void zu::type_checker::do_and_node(zu::and_node * const node, int lvl) {
  processBinaryExpressionInt(node, lvl);
}

void zu::type_checker::do_or_node(zu::or_node * const node, int lvl) {
  processBinaryExpressionInt(node, lvl);
}

void zu::type_checker::do_not_node(zu::not_node * const node, int lvl) {
    processUnaryExpression(node, lvl);
}


void zu::type_checker::do_ret_node(zu::ret_node * const node, int lvl) {}

void zu::type_checker::do_cont_node(zu::cont_node * const node, int lvl) {}

void zu::type_checker::do_break_node(zu::break_node * const node, int lvl) {}

void zu::type_checker::do_function_impl_node(zu::function_impl_node * const node, int lvl) {
  offset=0;
  argOffset=-8;
  std::string id = *(node->identifier());
  std::string funcId = '$'+id;
  std::shared_ptr<symbol> sym = _symtab.find(funcId);
  std::vector<basic_type> b;
  std::vector<basic_type> b2;
  if (sym) {
    if (sym->declared()) throw "function already declared";
    else {
          _symtab.replace(funcId, std::make_shared<symbol>(new basic_type(*node->retType()), funcId, b, node->getPublic(), node->getExtern(), true));
    }
    if (sym->args().size() != node->statements()->nodes().size() && node->statements()->node(0) != nullptr ) throw "wrong number of arguments";
    if (sym->type()->name() != node->retType()->name()) throw "wrong return type";
    for (size_t i = -1; i < sym->args().size(); i++)
    {
      cdk::expression_node *n = dynamic_cast<cdk::expression_node*>(node->statements()->node(sym->args().size()-i));
      if (sym->args()[sym->args().size()-i].name() != n->type()->name() && node->statements()->node(0) != nullptr)
       throw "wrong type in arguments";
    }

    _symtab.push(); 

    if (node->statements()->node(0) != nullptr){
      for (size_t i = 0; i < node->statements()->size(); i++){
        zu::declaration_node* n = dynamic_cast<zu::declaration_node*>(node->statements()->node(node->statements()->size()-1-i));
        std::shared_ptr<symbol> sym = _symtab.find_local(*n->identifier()->name());
        if (sym) throw "variable already declared";
        _symtab.insert(*n->identifier()->name(), std::make_shared<symbol>(new basic_type(*n->type()), *n->identifier()->name(), false, false, argOffset));        
        argOffset -= n->type()->size();
      }
    }    

    if (node->retType()->name() != basic_type::TYPE_VOID) { //FIXME
      _symtab.insert(id, std::make_shared<symbol>(new basic_type(*node->retType()), id, false, false, 0));
    }

  } else {
    if (node->statements()->node(0) != nullptr){
      for (size_t i = 0; i < node->statements()->size(); i++){
        zu::declaration_node* n = dynamic_cast<zu::declaration_node*>(node->statements()->node(node->statements()->size()-1-i));
        b.push_back(*n->type());
      }
    } 
    _symtab.insert(funcId, std::make_shared<symbol>(new basic_type(*node->retType()), funcId, b, node->getPublic(), node->getExtern(), true));
    
    _symtab.push(); 

    if (node->statements()->node(0) != nullptr){
      for (size_t i = 0; i < node->statements()->size(); i++){
        zu::declaration_node* n = dynamic_cast<zu::declaration_node*>(node->statements()->node(node->statements()->size()-1-i));
        std::shared_ptr<symbol> sym = _symtab.find_local(*n->identifier()->name());
        if (sym) throw "variable already declared";
        _symtab.insert(*n->identifier()->name(), std::make_shared<symbol>(new basic_type(*n->type()), *n->identifier()->name(), false, false, argOffset));        
        argOffset -= n->type()->size();
      }
    }    

    if (node->retType()->name() != basic_type::TYPE_VOID) { //FIXME
      _symtab.insert(id, std::make_shared<symbol>(new basic_type(*node->retType()), id, false, false, 0));
    }
  } 
  if (node->retValue() != nullptr){
    node->retValue()->accept(this, lvl+4);
     if(node->retValue()->type()->name() != node->retType()->name() && !(node->retValue()->type()->name() == basic_type::TYPE_INT && node->retType()->name() == basic_type::TYPE_DOUBLE)) 
      throw "return type and return value do not match";  
  }

}

void zu::type_checker::do_function_call_node(zu::function_call_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->statements()->accept(this, lvl);
  std::string id = '$' + *(node->identifier());
  std::shared_ptr<symbol> sym = _symtab.find(id);
  if (!sym) throw std::string(id + " undeclared");


  if (!dynamic_cast<cdk::nil_node*>(node->statements()->node(0))){
    if (sym->args().size() != node->statements()->nodes().size()) throw "wrong number of arguments";
    for (size_t i = 0; i < sym->args().size(); i++){
      node->statements()->node(sym->args().size()-1-i)->accept(this,lvl);
      cdk::expression_node *n = dynamic_cast<cdk::expression_node*>(node->statements()->node(sym->args().size()-1-i));
      if (n->type()->name() != sym->args()[sym->args().size()-1-i].name() && \
              !(n->type()->name() == basic_type::TYPE_INT && sym->args()[sym->args().size()-1-i].name() == basic_type::TYPE_DOUBLE)){ 
        throw n->type()->name() + "wrong type";
      }
    }
  }
  node->type(sym->type());
}

void zu::type_checker::do_function_dec_node(zu::function_dec_node * const node, int lvl) {
  std::string id = '$' + *(node->identifier());
  std::vector<basic_type> b;

  if (node->statements()->node(0) != nullptr){
    for (size_t i = 0; i < node->statements()->size(); i++){
      zu::declaration_node* n = dynamic_cast<zu::declaration_node*>(node->statements()->node(node->statements()->size()-1-i));
      b.push_back(*n->type());
    }
  } 
  if (!_symtab.insert(id, std::make_shared<symbol>(new basic_type(*node->retType()), id, b, node->getPublic(), node->getExtern(), false)))
    throw id + " redeclared";
}


void zu::type_checker::do_identifier_node(zu::identifier_node * const node, int lvl) {
  ASSERT_UNSPEC;
  std::string id = *(node->name());
  std::shared_ptr<symbol> sym = _symtab.find(id);
  if (sym == nullptr) throw id + " undeclared id";
  node->type(sym->type());
}


void zu::type_checker::do_declaration_node(zu::declaration_node * const node, int lvl) {
  std::string id = *(node->identifier()->name());
  offset += node->type()->size();
  if (!_symtab.insert(id, std::make_shared<symbol>(new basic_type(*node->type()), id, node->getPublic(), node->getExtern(), offset)))
    throw id + " redeclared";
 
  node->identifier()->accept(this, lvl+4);
  
  if (node->rvalue()!= nullptr) {
    node->rvalue()->accept(this, lvl+4);
    if (dynamic_cast<cdk::integer_node*>(node->rvalue())){
      if (node->type()->name() == basic_type::TYPE_POINTER && (dynamic_cast<cdk::integer_node*>(node->rvalue()))->value()==0){

      }else if (node->type()->name() != node->rvalue()->type()->name() && !(node->type()->name() == basic_type::TYPE_DOUBLE && node->rvalue()->type()->name() == basic_type::TYPE_INT))
        throw std::string("wrong type for initializer");
    }else if (node->type()->name() != node->rvalue()->type()->name() && !(node->type()->name() == basic_type::TYPE_DOUBLE && node->rvalue()->type()->name() == basic_type::TYPE_INT))
      throw std::string("wrong type for initializer");
  }
  node->type(node->identifier()->type());
}

void zu::type_checker::do_identity_node(zu::identity_node * const node, int lvl) {
  processUnaryExpression(node, lvl);
}

void zu::type_checker::do_malloc_node(zu::malloc_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->arg()->accept(this, lvl);
  if (node->arg()->type()->name() != basic_type::TYPE_INT)
    throw "malloc size should be an int";

  node->type(new basic_type(4, basic_type::TYPE_POINTER));
  node->type()->_subtype=(new basic_type(4, basic_type::TYPE_UNSPEC));
}

void zu::type_checker::do_address_node(zu::address_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->lval()->accept(this, lvl + 4);
  node->type(new basic_type(4, basic_type::TYPE_POINTER));
  node->type()->_subtype = node->lval()->type();
}

void zu::type_checker::do_index_node(zu::index_node * const node, int lvl) {
  ASSERT_UNSPEC;
  node->index()->accept(this, lvl+4);
  if (node->index()->type()->name() != basic_type::TYPE_INT)
    throw "wrong type in index";
  else
  node->pointer()->accept(this, lvl+4);

  node->type(node->pointer()->type()->subtype());
}

void zu::type_checker::do_block_node(zu::block_node * const node, int lvl) {
  if(node->declarationBlock() != nullptr)
    node->declarationBlock()->accept(this,lvl+4);
  if(node->instructionBlock() != nullptr)
    node->instructionBlock()->accept(this,lvl+4);
}


//---------------------------------------------------------------------------

void zu::type_checker::do_if_node(zu::if_node * const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  _symtab.push();
}

void zu::type_checker::do_if_else_node(zu::if_else_node * const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
  node->thenblock()->accept(this, lvl + 4);
  node->elseblock()->accept(this, lvl + 4);
}

void zu::type_checker::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

void zu::type_checker::do_lvalue_node(zu::lvalue_node * const node, int lvl){}

bool zu::type_checker::checkPointer(basic_type *b1, basic_type *b2){
  while (b1->name() != basic_type::TYPE_UNSPEC && b2->name() != basic_type::TYPE_UNSPEC && (!b1 && !b2)){
    if (b1->subtype() == b2->subtype()){
      b1 = b1->subtype();
      b2 = b2->subtype();
    } else return false;
    if (!b1 && !b2) return true;
  }
  return true;
}

