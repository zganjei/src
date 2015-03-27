
%{

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <list>

class expr;
  class Linear_Expression;
  class Preorder;
  class Rule;
  class Rules;


#include "parser.hh"

extern int line_no;

%}

%option noyywrap

ID   [A-Za-z_][A-Za-z_0-9]*
INT  [0-9]+
%x c_comment
%%

"//".*\n     /*Skip- comment :) */

"/*"			    { BEGIN(c_comment); };

<c_comment>"/*" 		    { fprintf(stderr, "Warning: Nested comments\n"); };
<c_comment>"*/"		    	    { BEGIN(INITIAL); } ;
<c_comment>. 			    /*Skip- comment :) */

{INT}		{ yylval.n = atoi(yytext); 
#ifdef print_read_debug
		  std::cout  <<yylval.n  << std::endl; 
#endif
		  return INT; 
       		  }

"vars" 		  { 
#ifdef print_read_debug
		      std::cout  << "vars"  << std::endl;
#endif
		      return  TOKEN_VARS; 
		  }

"aux" 		  { 
#ifdef print_read_debug
		      std::cout  << "aux"  << std::endl;
#endif
		      return  TOKEN_AUX; 
		  }

"initial"       { 
#ifdef print_read_debug
			std::cout  << "initial"  << std::endl;
#endif
		  	return  TOKEN_INITIAL; 
		}

"bad"       	{ 
#ifdef print_read_debug
			std::cout  << "bad"  << std::endl;
#endif
		  	return  TOKEN_BAD; 
		}

"top"       	{ 
#ifdef print_read_debug
			std::cout  << "top"  << std::endl;
#endif
		  	return  TOKEN_TOP; 
		}

"preceq"       	{ 
#ifdef print_read_debug
			std::cout  << "preceq"  << std::endl;
#endif
		  	return  TOKEN_PRECEQ; 
		}

"rules"        	{ 
#ifdef print_read_debug
			std::cout  << "rules"  << std::endl;
#endif
		  	return  TOKEN_RULES; 
		}

"'"             { 
#ifdef print_read_debug
			std::cout  << "'"  << std::endl;
#endif
		  	return  TOKEN_NEXT; 
		}

"("		{ 
#ifdef print_read_debug
			std::cout << "(" << std::endl;
#endif
		  	return TOKEN_LEFT_PARENTHESIS;
		}

")"		{ 
#ifdef print_read_debug
			std::cout << ")" << std::endl;
#endif
		  	return TOKEN_RIGHT_PARENTHESIS;
		}


"commands"      { 
#ifdef print_read_debug
			std::cout  << "commands"  << std::endl;
#endif
		  	return TOKEN_COMMANDS; 
		}

"check_reachability"	{ 
#ifdef print_read_debug
			 std::cout  << "check"  << std::endl;
#endif
		  	 return  TOKEN_CMD_CHECK_REACHABILITY; 
		}

"pred"  	{ 
#ifdef print_read_debug
			std::cout  << "predecessor"  << std::endl;
#endif
		  	return  TOKEN_CMD_PRED; 
		}

"up"		{ 
#ifdef print_read_debug
			std::cout  << "upward_closure"  << std::endl;
#endif
		  	return  TOKEN_CMD_UP; 
		}

"post"		{ 
#ifdef print_read_debug
			std::cout  << "post"  << std::endl;
#endif
		  	return  TOKEN_CMD_POST; 
		}
"*"		{ 
#ifdef print_read_debug
			std::cout  << "*"  << std::endl;
#endif
		  	return  TOKEN_MULTIPLY;
		}
"+"		{ 
#ifdef print_read_debug
			std::cout  << "+"  << std::endl;
#endif
		  	return  TOKEN_PLUS;
		}

"-"		{ 
#ifdef print_read_debug
			std::cout  << "-"  << std::endl;
#endif
		  	return  TOKEN_MINUS;
		}


">="		{ 
#ifdef print_read_debug
			std::cout  << ">="  << std::endl;
#endif
		  	return  TOKEN_GREAT_EQUAL;
		}

"<="		{ 
#ifdef print_read_debug
			std::cout  << "<="  << std::endl;
#endif
		  	return  TOKEN_LESS_EQUAL;
		}

">"		{ 
#ifdef print_read_debug
			std::cout  << ">"  << std::endl;
#endif
		  	return  TOKEN_GREAT;
		}

"<"		{ 
#ifdef print_read_debug
			std::cout  << "<="  << std::endl;
#endif
		  	return  TOKEN_LESS;
		}

"="		{ 
#ifdef print_read_debug
			std::cout  << "="  << std::endl;
#endif
		  	return  TOKEN_EQUAL;
		}

"==>"		{ 
#ifdef print_read_debug
			std::cout  << "==>"  << std::endl;
#endif
		  	return  TOKEN_IMPLY;
		}

":"		{ 
#ifdef print_read_debug
			std::cout  << ":"  << std::endl;
#endif
		  	return  TOKEN_COLON;
		}

";"		{ 
#ifdef print_read_debug
			std::cout  << ";"  << std::endl;
#endif
		  	return  TOKEN_SEMI_COLON;
		}

"with"		{ 
#ifdef print_read_debug
			std::cout  << "with"  << std::endl;
#endif
		  	return  TOKEN_WITH;
		}

"constrained"	{ 
#ifdef print_read_debug
			std::cout  << "constrained"  << std::endl;
#endif
		  	return  TOKEN_CONSTRAINED;
		}


"true"		{ 
#ifdef print_read_debug
			std::cout  << "true"  << std::endl;
#endif
		  	return  TOKEN_TRUE;
		}

"false"		{ 
#ifdef print_read_debug
			std::cout  << "false"  << std::endl;
#endif
		  	return  TOKEN_FALSE;
		}

"and"		{ 
#ifdef print_read_debug
			std::cout << "and" << std::endl;
#endif
		  	return TOKEN_AND;
		}

"or"		{ 
#ifdef print_read_debug
			std::cout << "or" << std::endl;
#endif
		  	return TOKEN_OR;
		}



[ \t\v\f\r\b]+  {/*skip*/ }

"#"[^\n]*\n 	{ ++line_no; }

[\n]            { ++line_no; }

{ID}            { yylval.str = new std::string(yytext); 		 
#ifdef print_read_debug
		  std::cout  << *yylval.str  << std::endl;
#endif
		  return ID; 
		  }

.               { std::cerr << "Parse error for " << yytext 
		  	    << " at line " << line_no << std::endl; 
			    }

%%
