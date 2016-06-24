// $Id: postfix_writer.cpp,v 1.48 2016/05/20 15:25:10 ist177983 Exp $ -*- c++ -*-
#include <string>
#include <sstream>
#include <iostream>
#include "targets/type_checker.h"
#include "targets/postfix_writer.h"
#include "targets/stack_counter.h"
#include "ast/all.h"  // all.h is automatically generated

//---------------------------------------------------------------------------
//     THIS IS THE VISITOR'S DEFINITION

//HOW TO RUN
//../zu zu.zu -o zu.asm
//yasm -felf32 zu.asm -o zu.o
//ld zu.o -o zu -m elf_i386 -L/home/joao/compiladores/root/usr/lib -lrts
//---------------------------------------------------------------------------

void zu::postfix_writer::writeMap() {
  std::map<std::string,std::string>::iterator it;
  for (it=ext.begin(); it!=ext.end(); ++it){
    _pf.EXTERN(it->second);
  }
}

void zu::postfix_writer::do_sequence_node(cdk::sequence_node * const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_integer_node(cdk::integer_node * const node, int lvl) {
  _pf.INT(node->value()); // push an integer
}

void zu::postfix_writer::do_double_node(cdk::double_node * const node, int lvl) {
  int lbl = ++_lbl;
  _pf.RODATA();
  _pf.ALIGN();
  _pf.LABEL(mklbl(lbl));
  _pf.DOUBLE(node->value());
  _pf.TEXT();
  _pf.ADDR(mklbl(lbl));
  _pf.DLOAD();
}

void zu::postfix_writer::do_string_node(cdk::string_node * const node, int lvl) {
  int lbl1;

  /* generate the string */
  _pf.RODATA(); // strings are DATA readonly
  _pf.ALIGN(); // make sure we are aligned
  _pf.LABEL(mklbl(lbl1 = ++_lbl)); // give the string a name
  _pf.STR(node->value()); // output string characters

  /* leave the address on the stack */
  _pf.TEXT(); // return to the TEXT segment
  _pf.ADDR(mklbl(lbl1)); // the string to be printed
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_neg_node(cdk::neg_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->argument()->accept(this, lvl); // determine the value
  if(node->type()->name() == basic_type::TYPE_INT){
    _pf.NEG(); // 2-complement
  } 
  else if(node->type()->name() == basic_type::TYPE_DOUBLE){
    _pf.DNEG(); // 2-complement
  }
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_add_node(cdk::add_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);

  if(node->type()->name() == basic_type::TYPE_INT){
    node->right()->accept(this, lvl);
    _pf.ADD(); return;
  }

  if ( node->left()->type()->name()== basic_type::TYPE_POINTER && node->right()->type()->name()== basic_type::TYPE_INT ){
    node->right()->accept(this, lvl);
    _pf.INT(node->type()->subtype()->size());
    _pf.MUL();
    _pf.ADD();
    return;
  } 
  else if ( node->right()->type()->name()== basic_type::TYPE_POINTER && node->left()->type()->name()== basic_type::TYPE_INT ){
    _pf.INT(node->type()->subtype()->size());
    _pf.MUL();
    node->right()->accept(this, lvl);
    _pf.ADD();
    return;
  } 

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }

  node->right()->accept(this, lvl);

  
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  if(node->left()->type()->name() == basic_type::TYPE_DOUBLE || node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.DADD();
  }

}

void zu::postfix_writer::do_sub_node(cdk::sub_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);

  node->left()->accept(this, lvl);
  if(node->type()->name() == basic_type::TYPE_INT){
    if (node->left()->type()->name() == basic_type::TYPE_POINTER && node->right()->type()->name() == basic_type::TYPE_POINTER){
      node->right()->accept(this, lvl);
      _pf.SUB();
      _pf.INT(node->left()->type()->subtype()->size());
      _pf.DIV();
      return;    
    }
    node->right()->accept(this, lvl);
    _pf.SUB(); return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);

  if ( node->left()->type()->name()== basic_type::TYPE_POINTER && node->right()->type()->name()== basic_type::TYPE_INT ){
    _pf.INT(node->type()->subtype()->size());
    _pf.MUL();
  } 

  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  if(node->left()->type()->name() == basic_type::TYPE_DOUBLE || node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.DSUB();
  }else{
    _pf.SUB();
  }
}
void zu::postfix_writer::do_mul_node(cdk::mul_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  if(node->left()->type()->name() == basic_type::TYPE_DOUBLE || node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.DMUL();
  }else{
    _pf.MUL();
  }
}
void zu::postfix_writer::do_div_node(cdk::div_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  if(node->left()->type()->name() == basic_type::TYPE_DOUBLE || node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.DDIV();
  }else{
    _pf.DIV();
  }
}

void zu::postfix_writer::do_mod_node(cdk::mod_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  node->right()->accept(this, lvl);
  _pf.MOD();
}

void zu::postfix_writer::do_lt_node(cdk::lt_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);

  if(node->type()->name() == basic_type::TYPE_INT){
    node->right()->accept(this, lvl);
    _pf.LT();
    return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }

  _pf.DCMP();
  _pf.INT(0);
  _pf.LT();


}

void zu::postfix_writer::do_le_node(cdk::le_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  
  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_INT ){
    node->right()->accept(this, lvl);
    _pf.LE();
    return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  _pf.DCMP();
  _pf.INT(0);
  _pf.LE();
}

void zu::postfix_writer::do_ge_node(cdk::ge_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  
  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_INT ){
    node->right()->accept(this, lvl);
    _pf.GE();
    return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  _pf.DCMP();
  _pf.INT(0);
  _pf.GE();
}
void zu::postfix_writer::do_gt_node(cdk::gt_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  
  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_INT ){
    node->right()->accept(this, lvl);
    _pf.GT();
    return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  _pf.DCMP();
  _pf.INT(0);
  _pf.GT();
}
void zu::postfix_writer::do_ne_node(cdk::ne_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  
  if(node->type()->name() == basic_type::TYPE_INT || node->type()->name() == basic_type::TYPE_POINTER ){
    node->right()->accept(this, lvl);
    _pf.NE();
    return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  _pf.DCMP();
  _pf.INT(0);
  _pf.NE();
}

void zu::postfix_writer::do_eq_node(cdk::eq_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->left()->accept(this, lvl);
  
  if(node->type()->name() == basic_type::TYPE_INT || node->type()->name() == basic_type::TYPE_POINTER ){
    node->right()->accept(this, lvl);
    _pf.EQ();
    return;
  }

  if(node->left()->type()->name() == basic_type::TYPE_INT && node->right()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  node->right()->accept(this, lvl);
  if(node->right()->type()->name() == basic_type::TYPE_INT && node->left()->type()->name() == basic_type::TYPE_DOUBLE ){
    _pf.I2D();
  }
  _pf.DCMP();
  _pf.INT(0);
  _pf.EQ();
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_rvalue_node(zu::rvalue_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->lvalue()->accept(this, lvl);
  if (node->type()->name() == basic_type::TYPE_DOUBLE)
   _pf.DLOAD();
  else 
    _pf.LOAD();

}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_lvalue_node(zu::lvalue_node * const node, int lvl) {
  //CHECK_TYPES(_compiler, _symtab, node);
  // FIXME simplified generation: all variables are global
  //_pf.ADDR(node->value());
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_assignment_node(zu::assignment_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->rvalue()->accept(this, lvl);
  std::string id;
  
  if (node->type()->name() == basic_type::TYPE_DOUBLE && node->rvalue()->type()->name() == basic_type::TYPE_INT) _pf.I2D(); // determine the new value
  if (node->type()->name() == basic_type::TYPE_DOUBLE)_pf.DDUP();
  else _pf.DUP();

  node->lvalue()->accept(this,lvl);
 
  if (node->lvalue()->type()->name() == basic_type::TYPE_DOUBLE){
    _pf.DSTORE();
  } else 
    _pf.STORE();

}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_function_impl_node(zu::function_impl_node * const node, int lvl) {
  // generate the main function (RTS mandates that its name be "_main")
  CHECK_TYPES(_compiler, _symtab, node);
  func = ++_lbl;
  global = false;
  std::string name = "";
  retOffset= node->retType()->size();
  stack_counter memory(_compiler);
  node->block()->accept(&memory, 0);
  size_t enterSize = memory.getCounter();
  ext.erase (*node->identifier()); 

  if (*node->identifier() == "zu") name = "_main";
  else if (*node->identifier() == "_main") name = "$zu";
  else name = '$'+*node->identifier();

  _pf.TEXT();
  _pf.ALIGN();
  _pf.GLOBAL(name, _pf.FUNC());
  _pf.LABEL(name);
  _pf.ENTER(enterSize + retOffset); 

  // end the main function
  if(node->retValue()!= nullptr){
    node->retValue()->accept(this, lvl+2);
    _pf.LOCA(-retOffset);
  }

  node->block()->accept(this, lvl);
  _pf.LABEL(mklbl(func));
  //verify types
  if (node->retType()->name() != basic_type::TYPE_VOID){
    _pf.LOCAL(-retOffset);
    if(node->retType()->name() == basic_type::TYPE_DOUBLE){
      if (node->retValue()!= nullptr && node->retValue()->type()->name() == basic_type::TYPE_INT){
        _pf.LOAD();
        _pf.I2D();
        _pf.DPOP();
      } else{
        _pf.DLOAD();
        _pf.DPOP();
      }
    }
    else{
      _pf.LOAD();
      _pf.POP();
    } 
  }

  _pf.LEAVE();
  _pf.RET();
  _symtab.pop();
  global = true;

  // these are just a few library function imports


}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_evaluation_node(zu::evaluation_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->argument()->accept(this, lvl); // determine the value
  if (node->argument()->type()->name() == basic_type::TYPE_INT || node->argument()->type()->name() == basic_type::TYPE_DOUBLE || node->argument()->type()->name() == basic_type::TYPE_POINTER || node->argument()->type()->name() == basic_type::TYPE_STRING || node->argument()->type()->name() == basic_type::TYPE_VOID) {
    _pf.TRASH(node->argument()->type()->size()); // delete the evaluated value
  } else {
    std::cerr << "ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
}

void zu::postfix_writer::do_print_node(zu::print_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->argument()->accept(this, lvl); // determine the value to print

  if (node->argument()->type()->name() == basic_type::TYPE_INT || node->argument()->type()->name() == basic_type::TYPE_POINTER) {
    _pf.CALL("printi");
    std::string x = "printi";
    ext.insert(std::pair<std::string,std::string>(x,x));
    _pf.TRASH(4); // delete the printed value
  }else if (node->argument()->type()->name() == basic_type::TYPE_DOUBLE) {
    _pf.CALL("printd");
    std::string x = "printd";
    ext.insert(std::pair<std::string,std::string>(x,x));
    _pf.TRASH(8); // delete the printed value FIXME
  }
  else if (node->argument()->type()->name() == basic_type::TYPE_STRING) {
    _pf.CALL("prints");
    std::string x = "prints";
    ext.insert(std::pair<std::string,std::string>(x,x));
    _pf.TRASH(4); // delete the printed value's address
  } else {
    std::cerr << node->argument()->type()->name() <<"ERROR: CANNOT HAPPEN!" << std::endl;
    exit(1);
  }
  if(node->ln()){
    _pf.CALL("println"); // print a newline
    std::string x = "println";
    ext.insert(std::pair<std::string,std::string>(x,x));
  }
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_read_node(zu::read_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  if(node->type()->name() == basic_type::TYPE_INT){
    _pf.CALL("readi");
    std::string x = "readi";
    ext.insert(std::pair<std::string,std::string>(x,x));
    _pf.PUSH();
  } else {
    _pf.CALL("readd");
    std::string x = "readd";
    ext.insert(std::pair<std::string,std::string>(x,x));
    _pf.DPUSH();
  }
}

//---------------------------------------------------------------------------
//New
void zu::postfix_writer::do_for_node(zu::for_node * const node, int lvl) {
  int test = ++_lbl, incrBreak = ++_lbl, incrCont = ++_lbl,endFor = ++_lbl;
  brk.push(incrBreak);
  cont.push(incrCont);

  node->init()->accept(this,lvl);
  _pf.LABEL(mklbl(test));
  for (size_t i = 0; i<node->condition()->nodes().size()-1; i++){
    node->condition()->node(i)->accept(this,lvl);
    _pf.TRASH(dynamic_cast<cdk::expression_node*>(node->condition()->node(i))->type()->size());
  }
  node->condition()->node(node->condition()->nodes().size()-1)->accept(this,lvl);
  
  _pf.JZ(mklbl(endFor));
  node->block()->accept(this,lvl);
  node->incr()->accept(this,lvl);
  _pf.JMP(mklbl(test));
  _pf.LABEL(mklbl(incrCont)); //for cont
  node->incr()->accept(this,lvl);
  _pf.JMP(mklbl(test));
  _pf.LABEL(mklbl(incrBreak)); //for break
  node->incr()->accept(this,lvl);
  _pf.LABEL(mklbl(endFor));

  cont.pop();
  brk.pop();
}

void zu::postfix_writer::do_break_node(zu::break_node * const node, int lvl) {
  _pf.JMP(mklbl(brk.top()));
}

void zu::postfix_writer::do_cont_node(zu::cont_node * const node, int lvl) {
  _pf.JMP(mklbl(cont.top()));
}

void zu::postfix_writer::do_and_node(zu::and_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() == basic_type::TYPE_DOUBLE) _pf.DDUP();
  else _pf.DUP();
  _pf.JZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.AND();
  _pf.LABEL(mklbl(lbl));
}

void zu::postfix_writer::do_or_node(zu::or_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  int lbl = ++_lbl;
  node->left()->accept(this, lvl);
  if (node->left()->type()->name() == basic_type::TYPE_DOUBLE) _pf.DDUP();
  else _pf.DUP();
  _pf.JNZ(mklbl(lbl));
  node->right()->accept(this, lvl);
  _pf.OR(); 
  _pf.LABEL(mklbl(lbl));
}

void zu::postfix_writer::do_not_node(zu::not_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->argument()->accept(this, lvl);
  _pf.NOT();
}


void zu::postfix_writer::do_ret_node(zu::ret_node * const node, int lvl) {
  _pf.JMP(mklbl(func));
}

void zu::postfix_writer::do_identity_node(zu::identity_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);

  if(node->type()->name() == basic_type::TYPE_INT){
    dynamic_cast<cdk::integer_node*>(node->argument())->accept(this, lvl);
  } 
  else if(node->type()->name() == basic_type::TYPE_DOUBLE){

    dynamic_cast<cdk::double_node*>(node->argument())->accept(this, lvl);

  }
}

void zu::postfix_writer::do_function_call_node(zu::function_call_node * const node, int lvl){
  CHECK_TYPES(_compiler, _symtab, node);
  std::string name = '$' + *node->identifier();
  std::shared_ptr<zu::symbol> symbol = _symtab.find(name);

  if (!symbol->declared()){
    std::string x = *node->identifier();
    ext.insert(std::pair<std::string,std::string>(x,x));
  }

  if (node->statements()->node(0) != nullptr && symbol->args().size() > 0){
    for (size_t i = 0; i < node->statements()->size(); i++){
      cdk::expression_node *n = dynamic_cast<cdk::expression_node*>(node->statements()->node(i));
      if (symbol->args()[symbol->args().size()-1-i].name() == n->type()->name()){
        n->accept(this, lvl);
      }
      else if (n->type()->name() == basic_type::TYPE_INT && symbol->args()[symbol->args().size()-1-i].name() == basic_type::TYPE_DOUBLE) {
        int lbl = ++_lbl;
        _pf.RODATA();
        _pf.ALIGN();
        _pf.LABEL(mklbl(lbl));
        _pf.DOUBLE(dynamic_cast<cdk::integer_node*>(node->statements()->node(i))->value());
        _pf.TEXT();
        _pf.ADDR(mklbl(lbl));
        _pf.DLOAD();
      }
    }
  } 

  _pf.CALL(name);
  

  if (node->statements()->node(0) != nullptr && symbol->args().size() > 0){
    for (size_t i = 0; i < node->statements()->size(); i++){
      cdk::expression_node *n = dynamic_cast<cdk::expression_node*>(node->statements()->node(i));
      if (symbol->args()[symbol->args().size()-1-i].name() == n->type()->name()){
        _pf.TRASH(n->type()->size());
      }
      else if (n->type()->name() == basic_type::TYPE_INT && symbol->args()[symbol->args().size()-1-i].name() == basic_type::TYPE_DOUBLE) {
        _pf.TRASH(8);
      }
    }
  }

  if (symbol->type()->name() != basic_type::TYPE_VOID){
    if (symbol->type()->name() == basic_type::TYPE_DOUBLE){
      _pf.DPUSH();
    } else _pf.PUSH();
  }
}

void zu::postfix_writer::do_function_dec_node(zu::function_dec_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node); 
  std::string x = *node->identifier();
  if (node->getExtern()) ext.insert(std::pair<std::string,std::string>(x,x));
}

void zu::postfix_writer::do_address_node(zu::address_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  std::string id;
  if (dynamic_cast<zu::identifier_node*>(node->lval())){
    dynamic_cast<zu::identifier_node*>(node->lval())->accept(this,lvl);
  } else{
    dynamic_cast<zu::index_node*>(node->lval())->accept(this,lvl);
  }
}


void zu::postfix_writer::do_malloc_node(zu::malloc_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  node->arg()->accept(this, lvl); // malloc arg value
  _pf.INT(node->type()->subtype()->size()); // malloc suntype size
  _pf.MUL();
  _pf.ALLOC();
  _pf.SP();
}

void zu::postfix_writer::do_index_node(zu::index_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);

  node->pointer()->accept(this,lvl);
  if (dynamic_cast<zu::lvalue_node*>(node->pointer())){
    if (node->pointer()->type()->name() == basic_type::TYPE_DOUBLE) _pf.DLOAD();
    else _pf.LOAD();
  }

  cdk::expression_node* index = node->index();
  index->accept(this, lvl);


  _pf.INT(node->pointer()->type()->subtype()->size());
  _pf.MUL();
  _pf.ADD();
}

void zu::postfix_writer::do_identifier_node(zu::identifier_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  std::string id = *node->name();
  if (global){
    _pf.ADDR(id);
  } else {
    std::shared_ptr<zu::symbol> symbol = _symtab.find_local(id);
    if (!symbol) {
      _pf.ADDR(id);
    } else {
      if (static_cast<int>(symbol->offset()) >= 0) 
        _pf.LOCAL(-retOffset-symbol->offset());
      else if (static_cast<int>(symbol->offset()) < 0) 
        _pf.LOCAL(-symbol->offset());
    }
  }
}

void zu::postfix_writer::do_declaration_node(zu::declaration_node * const node, int lvl) {
  CHECK_TYPES(_compiler, _symtab, node);
  int lbl2 = ++_lbl;
  std::string id = *node->identifier()->name();
  std::shared_ptr<zu::symbol> symbol = _symtab.find(id);
  if (global) {
    if (node->rvalue() == nullptr){
      _pf.DATA();
      _pf.ALIGN();
      if (node->getPublic())
        _pf.GLOBAL(id, _pf.OBJ());
      _pf.LABEL(id);
      if(node->type()->name() == basic_type::TYPE_DOUBLE)
        _pf.DOUBLE(0);
      else
        _pf.CONST(0);
    } else {
      if(node->type()->name() == basic_type::TYPE_INT || node->type()->name() == basic_type::TYPE_POINTER){
        _pf.DATA();
        _pf.ALIGN();
        if (node->getPublic())
          _pf.GLOBAL(id, _pf.OBJ());
        _pf.LABEL(id);
        _pf.CONST((dynamic_cast<cdk::integer_node*>(node->rvalue()))->value());
      }
      else if(node->type()->name() == basic_type::TYPE_STRING){
        _pf.RODATA();
        _pf.ALIGN();
        _pf.LABEL(mklbl(lbl2));
        _pf.STR((dynamic_cast<cdk::string_node*>(node->rvalue()))->value()); //FIXME 
        _pf.DATA();
        _pf.ALIGN();
        if (node->getPublic())
          _pf.GLOBAL(id, _pf.OBJ());
        _pf.LABEL(id);
        _pf.ID(mklbl(lbl2));
      }
      else if(node->type()->name() == basic_type::TYPE_DOUBLE){
        _pf.DATA();
        _pf.ALIGN();
        if (node->getPublic())
          _pf.GLOBAL(id, _pf.OBJ());
        _pf.LABEL(id);
        if (dynamic_cast<cdk::double_node*>(node->rvalue())){ _pf.DOUBLE(dynamic_cast<cdk::double_node*>(node->rvalue())->value()); }
        else{_pf.DOUBLE(dynamic_cast<cdk::integer_node*>(node->rvalue())->value());}
      }
    }
  } else{
    if (node->rvalue() == nullptr){

    } else {
      node->rvalue()->accept(this, lvl);
      if(node->type()->name() == basic_type::TYPE_INT || node->type()->name() == basic_type::TYPE_POINTER){
        _pf.LOCA(-retOffset-symbol->offset());
      }
      else if(node->type()->name() == basic_type::TYPE_STRING){
        _pf.LOCA(-retOffset-symbol->offset());
      }
      else if(node->type()->name() == basic_type::TYPE_DOUBLE){
        _pf.LOCAL(-retOffset-symbol->offset());
        _pf.DSTORE();
      } else {

      }
    }
  }

}


void zu::postfix_writer::do_block_node(zu::block_node * const node, int lvl) {
  if (node->declarationBlock() != nullptr) node->declarationBlock()->accept(this, lvl+4);
  if (node->instructionBlock() != nullptr) node->instructionBlock()->accept(this, lvl+4);
}


//---------------------------------------------------------------------------

void zu::postfix_writer::do_if_node(zu::if_node * const node, int lvl) {
  int lbl1;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->block()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1));
  _symtab.pop();
}

//---------------------------------------------------------------------------

void zu::postfix_writer::do_if_else_node(zu::if_else_node * const node, int lvl) {
  int lbl1, lbl2;
  node->condition()->accept(this, lvl);
  _pf.JZ(mklbl(lbl1 = ++_lbl));
  node->thenblock()->accept(this, lvl + 2);
  _pf.JMP(mklbl(lbl2 = ++_lbl));
  _pf.LABEL(mklbl(lbl1));
  node->elseblock()->accept(this, lvl + 2);
  _pf.LABEL(mklbl(lbl1 = lbl2));
}
