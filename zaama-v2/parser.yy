%{

#include <iostream>
#include <assert.h>
#include <memory>
#include <fstream>

#include <string>
#include <list>
#include <map>
#include <vector>
#include <iterator>
#include <iostream>
#include <stdlib.h>

#include "z3++.h"

#include "linear_expression.hpp"
#include "fw_constraint.hpp"
#include "bw_constraint.hpp"
#include "rule.hpp"
#include "preorder.hpp"

#include "trace.hpp"
#include "sequence.hpp"
#include "fixpoint.hpp"
#include "cplex.hpp"



  using namespace z3;
  
  Preorder::sptr g_preorder;
  
  //Linear_Expressions
  z3::context Linear_Expression::cxt;
  //  unsigned Expression::variables_count;
  std::vector<std::string> Linear_Expression::vars_names;
  std::vector<z3::expr> Linear_Expression::vars; 
  std::vector<Z3_ast> Linear_Expression::vars_ast; 
  std::vector<z3::expr> Linear_Expression::primed_vars; 
  std::vector<Z3_ast> Linear_Expression::primed_vars_ast;
  std::vector<std::string> Linear_Expression::aux_vars_names;
  std::vector<z3::expr> Linear_Expression::aux_vars; 
  z3::tactic Linear_Expression::qe(z3::tactic(Linear_Expression::cxt,"qe"));
  z3::tactic Linear_Expression::simplify(z3::tactic(Linear_Expression::cxt,"simplify"));
  z3::tactic Linear_Expression::propagate(z3::tactic(Linear_Expression::cxt,"propagate-ineqs"));
  z3::tactic Linear_Expression::nnf(z3::tactic(Linear_Expression::cxt,"nnf"));
  z3::tactic Linear_Expression::skip(z3::tactic(Linear_Expression::cxt,"skip"));
  z3::tactic Linear_Expression::split_clause(z3::tactic(Linear_Expression::cxt,"split-clause"));
  z3::tactic Linear_Expression::eq2ineq(Linear_Expression::cxt, "simplify");
  z3::tactic Linear_Expression::cnf(Linear_Expression::cxt, "tseitin-cnf");
  z3::expr Linear_Expression::top_expr(Linear_Expression::cxt);
  z3::expr Linear_Expression::identity_expr(Linear_Expression::cxt);
  
  //cplex
  IloEnv Cplex::env;
  IloIntVarArray Cplex::vars(Cplex::env);
  IloIntVarArray Cplex::aux_vars(Cplex::env);
  std::vector<std::string> Cplex::vars_names;
  std::vector<std::string> Cplex::aux_vars_names;
 


  //Constraint
  unsigned Constraint::counter=0;


  //helpers for defining expressions
  std::set<unsigned> domain;
  std::set<unsigned> codomain;
  std::set<unsigned> auxdomain;

  extern int yylex();
  extern int line_no;
  extern char* yytext;
  
  void yyerror(const char* s);
  
  %}

%union {
  int n;
  std::string* str;
  expr* z3_expression;
  std::list<expr*>* z3_expressions;
  Linear_Expression* expression;
  Preorder* preceq;
  Rule* rule;
  Rules* rules; 
}


%token TOKEN_VARS
%token TOKEN_AUX
%token TOKEN_INITIAL
%token TOKEN_BAD
%token TOKEN_TOP
%token TOKEN_PRECEQ
%token TOKEN_RULES
%token TOKEN_NEXT
%token TOKEN_COMMANDS
%token TOKEN_CMD_CHECK_REACHABILITY
%token TOKEN_CMD_PRED
%token TOKEN_CMD_UP
%token TOKEN_CMD_POST
%token TOKEN_WITH
%token TOKEN_CONSTRAINED

%token TOKEN_MULTIPLY
%token TOKEN_PLUS
%token TOKEN_MINUS

%token TOKEN_GREAT_EQUAL
%token TOKEN_LESS_EQUAL
%token TOKEN_GREAT
%token TOKEN_LESS
%token TOKEN_EQUAL
%token TOKEN_IMPLY
%token TOKEN_COLON
%token TOKEN_SEMI_COLON

%token TOKEN_LEFT_PARENTHESIS
%token TOKEN_RIGHT_PARENTHESIS
%token TOKEN_AND
%token TOKEN_OR

%token TOKEN_TRUE
%token TOKEN_FALSE

%token <str> ID
%token <n> INT 

%type <z3_expression> z3_expression atom 
%type <z3_expressions> z3_expressions atoms
%type <expression> expression 
%type <preceq> preceq
%type <rule> rule
%type <rules> rules
%%

zaama: initialize TOKEN_VARS TOKEN_COLON variables TOKEN_AUX TOKEN_COLON aux_variables 
TOKEN_TOP TOKEN_COLON z3_expression 
{

  Linear_Expression::top_expr = *$10;
  
  assert(Linear_Expression::vars_names.size()==Linear_Expression::vars.size());
  assert(Linear_Expression::vars_ast.size()==Linear_Expression::vars.size());
  assert(Linear_Expression::primed_vars.size()==Linear_Expression::vars.size());
  assert(Linear_Expression::primed_vars_ast.size()==Linear_Expression::vars.size());

  assert(!Linear_Expression::vars.empty());

  Linear_Expression::identity_expr= z3::expr(Linear_Expression::vars[0] == Linear_Expression::primed_vars[0]);
  for(unsigned i=1; i<Linear_Expression::vars.size(); i++){

    Linear_Expression::identity_expr= z3::expr(Linear_Expression::identity_expr
					&& (Linear_Expression::vars[i] == Linear_Expression::primed_vars[i]));
					
	Linear_Expression::cxt.set(":pp-min-alias-size", 1000000);//*********** for preventing Z3 pretty printer (i.e. not printing let (....))
	Linear_Expression::cxt.set(":pp-max-depth", 1000000);//*********** for preventing Z3 pretty printer (i.e. not printing let (....))
					

   
  }
delete $10;//FIXME!
} TOKEN_COMMANDS TOKEN_COLON commands{}
;

initialize: //it doesn't match to any input, just performs some initialization stuff
{

  z3::params p(Linear_Expression::cxt); 
  p.set(":eq2ineq",true);
  Linear_Expression::eq2ineq= with(z3::tactic(Linear_Expression::cxt, "simplify"), p);

  // z3::params peq(Expression::cxt); 
  // peq.set(":eliminate-variables-as-block",false);
  // Expression::qe= with(z3::tactic(Expression::cxt, "qe"), peq);

} ;

variables: ID
{
  //  Expression::variables_count++;
  Linear_Expression::vars_names.push_back(*$1);
  Linear_Expression::vars.push_back(Linear_Expression::cxt.int_const($1->c_str()));
  Linear_Expression::vars_ast.push_back(Linear_Expression::cxt.int_const($1->c_str()));
   const char* name = (*$1).c_str();
  std::string primed= $1->append("_p");
  Linear_Expression::primed_vars.push_back(Linear_Expression::cxt.int_const(primed.c_str()));
  Linear_Expression::primed_vars_ast.push_back(Linear_Expression::cxt.int_const(primed.c_str()));
  
  //cplex
  Cplex::vars.add(IloIntVar(Cplex::env, 0 /*min value of var*/,IloIntMax,name));
   Cplex::vars_names.push_back(name);
  delete $1;
}
| variables ID {
  //  Expression::variables_count++;
  Linear_Expression::vars_names.push_back(*$2);
  Linear_Expression::vars.push_back(Linear_Expression::cxt.int_const($2->c_str()));
  Linear_Expression::vars_ast.push_back(Linear_Expression::cxt.int_const($2->c_str()));
   const char* name = (*$2).c_str();
  std::string primed= $2->append("_p");
  Linear_Expression::primed_vars.push_back(Linear_Expression::cxt.int_const(primed.c_str()));
  Linear_Expression::primed_vars_ast.push_back(Linear_Expression::cxt.int_const(primed.c_str()));
  
  //cplex 
  Cplex::vars.add(IloIntVar(Cplex::env, 0 /*min value of var*/,IloIntMax,name));
   Cplex::vars_names.push_back(name);
  delete $2;
}
;

aux_variables: 
{
}
| aux_variables ID {
  Linear_Expression::aux_vars_names.push_back(*$2);
   const char* name = (*$2).c_str();
  Linear_Expression::aux_vars.push_back(Linear_Expression::cxt.int_const($2->c_str()));
  
  //cplex
  Cplex::aux_vars.add(IloIntVar(Cplex::env, 0 /*min value of var*/,IloIntMax,name));
  Cplex::aux_vars_names.push_back(name);
  
  delete $2;
}
;
//#################

commands: command
{
  // Nothing
}
| command TOKEN_SEMI_COLON commands
{
}
;


command: expression TOKEN_IMPLY expression
{
 // std::cout << *$1 << "\n"   
//	    << "does " << (($1->implies(*$3))? " ": "not ") 
//	    << "imply : " << "\n"
//	    << *$3 << "\n";		
  
  delete $1;
  delete $3;
}
| TOKEN_CMD_CHECK_REACHABILITY TOKEN_COLON
TOKEN_INITIAL TOKEN_COLON expression
TOKEN_BAD TOKEN_COLON expression 
preceq
TOKEN_RULES TOKEN_COLON rules
{
//Preorder::sptr preceq($9);

Preorder::sptr preceq = g_preorder;


  Rules  rules(*$12);
  
  Sequence::sptr sequence; 

  
  //BW_Constraint::MSet bads= BW_Constraint::get_constraints(*$8);///XXX
 
  BW_Constraint::MSet bads;
   for(Linear_Expression lexpr: (*$8).split())//**
   		bads.insert(BW_Constraint::get_constraints(lexpr));
   		
  std::cout << "checking reachability of : " << "\n";
   std::cout << "bads: \n";  
    std::cout << bads << "\n";
  // getchar();
  
   FW_Constraint::MSet inits;
   for(Linear_Expression lexpr: (*$5).split())
   		inits.insert(FW_Constraint::sptr(new FW_Constraint(lexpr)));
   		    
  std::cout << "inits: [\n";
    std::cout << inits<< "\n";
    std::cout << "]\n";
 // getchar();  
  
  for(Rules::iterator rule=rules.begin();rule!=rules.end();){
  (*rule)->find_compatible_keys(preceq);
   		++rule;  
  }

  std::cout<<"with this rules:\n";
  
  for(Rule::sptr r: rules){  
    std::cout << r << "\n";
   
    }

  std::cout << "and preorder: " << preceq  << "\n";


 Fixpoint fixpoint(inits, rules, bads, preceq);
  tribool result=fixpoint.isReachable(sequence);
  std::cout << (result? "reachable" : "Not Reachable :)") << "\n\n";
  assert(sequence);
  if(!sequence->isEmpty())
    std::cout << "sequence " << sequence << "\n";     
  
  std::ofstream zaamaseq("zaama-counter-example.txt");

  zaamaseq << sequence;

  zaamaseq.close();  

  delete $12;
  
  

}
| TOKEN_CMD_PRED expression TOKEN_WITH rule
{
 
 /* for(Expression e: $2->split())
    cstrs.insert(Constraint::sptr(new Constraint(e,g_preorder->get_key(e))));*/ 
/* Constraint::MSet cstrs= g_preorder->get_constraints(*$2);
  Rule::sptr rule($4);

  std::cout << "pred of: ";
    std::cout << cstrs << "\n";
    
  std::cout << "with   : " 
	    << rule
	    << "\ngives  : " 
	    << rule->pred(cstrs) << "\n";*/
}
| TOKEN_CMD_POST expression TOKEN_WITH rule
{
  
  /*for(Expression e: $2->split())
    cstrs.insert(Constraint::sptr(new Constraint(e,g_preorder->get_key(e)))); ////FIXME*/
/*Constraint::MSet cstrs= g_preorder->get_constraints(*$2);
  Rule::sptr rule($4);

  std::cout << "post of: ";
    std::cout << cstrs << "\n";
  std::cout << "with   : " 
	    << rule
	    << "\ngives  : " 
	    << rule->post(cstrs) << "\n";*/
}
| TOKEN_CMD_UP expression TOKEN_WITH preceq
{

Preorder::sptr preceq($4);



  /*for(Expression e: $2->split())
    cstrs.insert(Constraint::sptr(new Constraint(e,preceq->get_key(e)))); */

   /* Constraint::MSet cstrs= preceq->get_constraints(*$2);

  std::cout << "upward closure of " << "\n"
	    << cstrs << "\n"
	    << "with " << "\n"
	    << preceq
	    << "is " << "\n"
	    << preceq->close(cstrs) << "\n";*/
}
;


//#################

rules: rule
{
  $$ = new Rules;
  $$->push_front(Rule::sptr($1));
  //delete $1;
}
| rules rule
{
  $$ = $1;
  $$->push_front(Rule::sptr($2));
  //delete $2;
}
;

rule: INT TOKEN_COLON  expression
{
  $$ = new Rule($1,  *$3);
  //delete $2;
  delete $3;
}
;

preceq: TOKEN_PRECEQ TOKEN_COLON expression
{
  $$ = new Preorder(*$3);
  delete $3;
} 
| TOKEN_CONSTRAINED TOKEN_PRECEQ  TOKEN_COLON expression TOKEN_COLON expression
{
  Preorder up(*$4);   
  std::vector<Linear_Expression> frontiers = $6->split();
  for(Linear_Expression f: frontiers){
    up.constrainWith(f);
    }
  $$ = new Preorder(up);
  g_preorder = Preorder::sptr($$);
  delete $4;
  delete $6;
} 
;

expression: {domain.clear(); codomain.clear(); auxdomain.clear();} z3_expression 
{ $$ = new Linear_Expression(*$2, domain, codomain, auxdomain);}

z3_expression: TOKEN_TRUE
{
  $$ = new z3::expr(Linear_Expression::cxt.bool_val(true));
}
| TOKEN_FALSE
{
  $$ = new z3::expr(Linear_Expression::cxt.bool_val(false));
}
| TOKEN_LEFT_PARENTHESIS z3_expression TOKEN_RIGHT_PARENTHESIS
{
  $$ = $2;
}
| TOKEN_LEFT_PARENTHESIS TOKEN_EQUAL atom atom TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr((*$3) == (*$4));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_EQUAL z3_expression z3_expression TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr((*$3) == (*$4));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_LESS_EQUAL atom atom TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr((*$3) <= (*$4));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_LESS atom atom TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr((*$3) < (*$4));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_GREAT_EQUAL atom atom TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr((*$3) >= (*$4));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_GREAT atom atom TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr((*$3) > (*$4));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_AND z3_expressions TOKEN_RIGHT_PARENTHESIS
{
  assert(!$3->empty());
  std::list<expr*>::const_iterator p,e= $3->begin();
  p = e; e++;
  $$ = new z3::expr(*(*p));
  for(; e != $3->end(); ++e){
    delete *p;
    p = e;
    *$$ = *$$ && *(*p);
  }
  delete $3;
}
| TOKEN_LEFT_PARENTHESIS TOKEN_OR z3_expressions TOKEN_RIGHT_PARENTHESIS
{
  assert(!$3->empty());
  std::list<expr*>::const_iterator p,e= $3->begin();
  p = e; e++;
  $$ = new z3::expr(*(*p));
  for(; e != $3->end(); ++e){
    delete *p;
    p = e;
    *$$ = *$$ || *(*p);
  }
  delete $3;
}
;

z3_expressions: z3_expression
{
  $$ = new std::list<expr*>;
  $$->push_back($1);
  //delete $1
}
| z3_expressions z3_expression
{
  $$=$1;
  $$->push_back($2);
  
  //delete $2;
}
;

atom: ID
{

  bool found= false;
  for(int i=0; i < Linear_Expression::vars_names.size(); ++i){
    if(Linear_Expression::vars_names[i].compare(*$1) == 0){
      $$ = new z3::expr(Linear_Expression::vars[i]);
      domain.insert(i);
      found = true;
      break;
    }
  }
  for(int i=0; !found && i < Linear_Expression::aux_vars_names.size(); ++i){
    if(Linear_Expression::aux_vars_names[i].compare(*$1) == 0){
      $$ = new z3::expr(Linear_Expression::aux_vars[i]);///XXXXXXXXXXXXXXXX
      auxdomain.insert(i);
      found = true;
      break;
    }
  }
  if(!found)
    yyerror("undeclared variable");
}
|ID TOKEN_NEXT
{
  int i=0;
  for(; i < Linear_Expression::vars_names.size(); ++i){
    if(Linear_Expression::vars_names[i].compare(*$1) == 0){
      $$ = new z3::expr(Linear_Expression::primed_vars[i]);
      codomain.insert(i);
      break;
    }
  }
  if(i == Linear_Expression::vars_names.size())
    yyerror("undeclared variable");
}
| INT
{
  $$ = new z3::expr(Linear_Expression::cxt.int_val($1));
}
| TOKEN_LEFT_PARENTHESIS atom TOKEN_RIGHT_PARENTHESIS
{
  $$ = $2;
}
| TOKEN_LEFT_PARENTHESIS TOKEN_MINUS INT TOKEN_RIGHT_PARENTHESIS
{
  $$ = new z3::expr(Linear_Expression::cxt.int_val($3 * (-1)));
}
| TOKEN_LEFT_PARENTHESIS TOKEN_MULTIPLY atoms TOKEN_RIGHT_PARENTHESIS
{
  assert(!$3->empty());
  std::list<expr*>::const_iterator p,e= $3->begin();
  p = e; e++;
  $$ = new z3::expr(*(*p));
  for(; e != $3->end(); ++e){
    delete *p;
    p = e;
    *$$ = *$$ * *(*p);
  }
  delete $3;
}
| TOKEN_LEFT_PARENTHESIS TOKEN_PLUS atoms TOKEN_RIGHT_PARENTHESIS
{
  assert(!$3->empty());
  std::list<expr*>::const_iterator p,e= $3->begin();
  p = e; e++;
  $$ = new z3::expr(*(*p));
  for(; e != $3->end(); ++e){
    delete *p;
    p = e;
    *$$ = *$$ + *(*p);
  }
  delete $3;
}
| TOKEN_LEFT_PARENTHESIS TOKEN_MINUS atoms TOKEN_RIGHT_PARENTHESIS
{
  assert(!$3->empty());
  std::list<expr*>::const_iterator p,e= $3->begin();
  p = e; e++;
  $$ = new z3::expr(*(*p));
  for(; e != $3->end(); ++e){
    delete *p;
    p = e;
    *$$ = *$$ - *(*p);
  }
  delete $3;
}
;

atoms: atom
{
  $$ = new std::list<expr*>;
  $$->push_back($1);
  //delete $1;
}
| atoms atom
{
  $$=$1;
  $$->push_back($2);
  //delete $2;
}
;

%%

void yyerror(const char* s)
{
  std::cerr << s << " at line " << line_no << " near " << yytext << "\n";
}
