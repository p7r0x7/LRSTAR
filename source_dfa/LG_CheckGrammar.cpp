
/* Copyright 2018 Paul B Mann.  BSD License. */

		#include "CM_Global.h"
	   #include "LG_Global.h"
		#include "LG_CheckGrammar.h"
  		#include "LG_Global.h"

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

int   LG_CheckGrammar::CheckGrammar ()
{
	  	if (optn[LG_VERBOSE] > 2) printf ("Checking grammar for problems ...\n");	  

      C_LENG ();
      C_NULLS ();																	
      C_HEADSYM ();																	
		CHECK_LEXICALS ();
		if (n_errors) return 0;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Checking for unreachable symbols ...\n");
      P_UNREACHABLES ();
		if (n_errors) return 0;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Checking for undefined symbols ...\n");
      P_UNDEFINED ();
		if (n_errors) return 0;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Checking for useless productions ...\n");
      P_USELESS_PROD ();
		if (n_errors) return 0;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Checking for null tokens ...\n");
      P_NULL_TOKENS ();
		if (n_errors) return 0;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Checking for unreducible symbols ...\n");
      P_UNREDUCIBLES ();
		if (n_errors) return 0;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Checking for cycles in grammar ...\n");
//		C_CYCLES();
		if (n_errors) return 0;

		FREE (head_sym, n_heads); // Free this, DO_BACK_SUB changes n_heads.

      if (optn[LG_VERBOSE])
			  prt_log     ("Grammar  %7d rules, %d terminals, %d nonterminals.\n", n_prods, max_char_set, n_heads);
      else prt_logonly ("Grammar  %7d rules, %d terminals, %d nonterminals.\n", n_prods, max_char_set, n_heads);

      PrintGrammar ();
		if (n_errors > 0) return (0);

		FREE (head_type, n_heads);
		FREE (head_line, n_heads);
		FREE (prod_type, n_prods);
		FREE (prod_line, n_prods);
		FREE (term_line, N_terms);

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Doing back-substitutions for null symbols ...\n");

	  	DO_BACK_SUB ();

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Done with back-substitutions !!!\n");
      if (optn[LG_VERBOSE] > 2) PrintGrammar (); // Printing grammar for 2nd time?
      return (1);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::C_LENG () /* Compute maximum symbol length. */
{
      max_headl = 35;
      max_terml = 35;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::C_NULLS ()
{
      int h, p, t, s, n;
      n_nulls = 0;
      ALLOC (nullable, n_heads);
      FASTINI (0, nullable, n_heads);
      do 
		{
         n = 0;
         for (h = 0; h < n_heads; h++)
         {
            if (nullable [h] == 0)
            {
               for (p = f_prod [h]; p < l_prod [h]; p++)
               {
                  for (t = f_tail [p]; t < l_tail [p]; t++)	/////////////////////////////////
                  {
                     if ((s = tail [t]) < 0)
                     {
                        if (nullable [-s] == 0) goto nextp;
                     }
                     else goto nextp;
                  }
                  n++;
                  n_nulls++;
                  nullable [h] = 1;
                  goto nexth;
nextp:            continue;
               }  
            }
nexth:      continue;
         }  
      }
      while (n > 0);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::C_HEADSYM ()
{
      int h, p;
      ALLOC (head_sym, n_prods);
      for (h = 0; h < n_heads; h++)
      {
         for (p = f_prod[h]; p < l_prod[h]; p++)
         {
            head_sym[p] = h;
         }  
      }
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::CHECK_LEXICALS ()
{
      int h;
      for (h = 0; h < n_heads; h++)
      {
         if (head_type[h] & LEXICON)
         {
				if (!(head_type[h] & OUTPUTSYM)) 
				{
               char* g;
               if (already_readlex) g = ".lex";
               else                 g = ".lgr";
					int x = (int)strlen (head_name[h])-1;
					head_name[h][x] = 0;
					sprintf (string, "<%s> assumed to be {%s} because it's not in the tokens list in %s%s", 
                  head_name[h]+1, head_name[h]+1, gfn, g);
					head_name[h][x] = '>';
					prt_warning (string, head_name[h], 0, head_line[h]);
               head_type[h] |= IGNORESYM;
				}
         }  
      }
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::P_UNDEFINED ()
{
      int t, n = n_errors;

		if (N_terms > max_char_set+1)
		{
			strcpy (gft, ".lex");
			strcpy (grmfid, gdn);
			strcat (grmfid, gfn);
			strcat (grmfid, gft);

			char*  Input_start = input_start;
			char*  Input_end   = input_end;
			char** Line_ptr    = line_ptr;

			input_start = lex_input_start;
			input_end   = lex_input_end;
			line_ptr    = lex_line_ptr;

			for (t = max_char_set+1; t < N_terms; t++) // Skip <eof>
			{
				if (term_type[t] & LEXFILE)
				prt_error ("'%s' is listed in the .lex file, but not defined in the .lgr file", term_name[t], 0, term_line[t]);
			}

			strcpy (gft, ".lgr");
			strcpy (grmfid, gdn);
			strcat (grmfid, gfn);
			strcat (grmfid, gft);

			input_start = Input_start;
			input_end   = Input_end;
			line_ptr    = Line_ptr;											

			for (t = max_char_set+1; t < N_terms; t++) // Skip <eof>
			{
				if (!(term_type[t] & LEXFILE))
				{
					char code = charcode[*term_name[t]];
					if ( code == DIGIT || code == QUOTE) 
					{
						prt_error ("'%s' is not a predefined symbol", term_name[t], 0, term_line[t]);
						prt_log ("Predefined symbols are:\n");
						prt_log ("   0..31\n");
						for (int i = 32; i < 127; i++) prt_log ("   %s\n", term_name[i]);
						prt_log ("   127..255\n");
						Quit ();
					}
					else
					{
						prt_error ("'%s' is not defined in the lexical grammar (.lgr file)", term_name[t], 0, term_line[t]);
					}
				}
			}
		}

		FREE (term_type, N_terms);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::P_NULL_TOKENS ()
{
      int p, t;
      for (p = f_prod [1]; p < l_prod [1]; p++)      // Token -> productions only. 
      {
         t = tail [f_tail [p]];                      // First tail symbol.
         if (t < 0)                                  // Nonterminal?        
         {
            if (nullable [-t])                       // Nullable?          
            {
					prt_error ("'%s' is a nullable symbol, lexer cannot return or ignore null symbols", head_name[-t], 0, head_line[-t]);
            }  
         }
      }  
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::P_USELESS_PROD ()
{
      int h, p;
		for (h = 1; h < n_heads; h++)
		{
			for (p = f_prod[h]; p < l_prod[h]; p++)
			{
				if ((l_tail[p] - f_tail[p]) == 1) 
				{
					if (h == -tail [f_tail[p]])
					{
						prt_log ("%s(%04d) : ", grmfid, prod_line[p]); 
						p_prod (-p, -1, "");
						prt_log ("\n");
						prt_error ("Useless production", 0, 0, prod_line[p]);
					}  
				}  
			}
		}
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::P_UNREACHABLES ()
{
      char *head_used;
      int  n_unreachables;
		int  *used_list, h, p, t, s, i, n_used, x;

      ALLOC (used_list, n_heads);
      ALLOC (head_used, n_heads);
      memset(head_used, 0, n_heads);

		i = 0;							// Goal symbol!
      n_used = 0;						// Start with 0. 
      head_used[i] = 1;				// Mark goal symbol used.
      used_list[n_used++] = i;	// Add goal symbol to list.

	// Traverse from goal symbol and mark all nonterminals that are used.
      for (; i < n_used; i++)
      {
         h = used_list[i];										// Pick head from list.
		//	printf ("%s\n", head_name[h]);
			for (p = f_prod[h]; p < l_prod[h]; p++)		// All of its rules. 
			{
				for (t = f_tail[p]; t < l_tail[p]; t++)	// All of its tails. 
				{
					if ((s = tail[t]) < 0)						// Nonterminal?      
					{
						if (head_used[-s] == 0)					// Not used?   
						{
							head_used[-s] = 1;					// Mark it used.     
							used_list[n_used++] = -s;			// Add it to list.   
						//	printf ("   %s used\n", head_name[-s]);
						}  
					}  
				}  
			}  
      }

	// Traverse from [ignore] symbols and mark all nonterminals that are used.
		int n_ignores = 0;
      for (int H = 0; H < n_heads; H++)
      {
			if (head_type[H] & IGNORESYM) 
			{
				n_ignores++;
			//	printf ("%s unused\n", head_name[H]);
				i = n_used;
				head_used[H] = 1;				// Mark ignore symbol used.
				used_list[n_used++] = H;	// Add ignore symbol to list.
				for (; i < n_used; i++)
				{
					h = used_list[i];										// Pick head from list.
				//	printf ("%s\n", head_name[h]);
					for (p = f_prod[h]; p < l_prod[h]; p++)		// All of its rules. 
					{
						for (t = f_tail[p]; t < l_tail[p]; t++)	// All of its tails. 
						{
							if ((s = tail[t]) < 0)						// Nonterminal?      
							{
								if (head_used[-s] == 0)					// Not marked yet?   
								{
									head_used[-s] = 1;				// Mark it used.     
									used_list[n_used++] = -s;		// Add it to list.   
								//	printf ("   %s used %d\n", head_name[-s], n_used);
								}  
							}  
						}  
					}  
				}
			}
		}
      FREE (used_list, n_heads);

	// List unreachable heads ...
		n_unreachables = 0;
      for (h = 0; h < n_heads; h++)		 
      {
			if (!(head_type[h] & IGNORESYM) && head_used[h] == 0) // Not [ignore] and not used?         
         {
				if (head_line[h] != 0)							// Not generated symbol?
				{
					if (head_type[h] & SETNAME)				// If setname (or escape symbol).
					{
						// Nothing. 
					}
					else if (head_type[h] & LEXICON)			// If lexicon symbol.
					{
						n_unreachables++;
						head_type[h] |= UNREACHABLE;
						x = (int)strlen(head_name[h]) -1;
						head_name[h][x] = 0;
						sprintf (string, "<%s> is not reachable from the goal symbol. To ignore, rename to [%s].\n", head_name[h]+1, head_name[h]+1);
						head_name[h][x] = '>';
						prt_error (string, head_name[h], 0, head_line[h]); 
					} 
					else
					{
						n_unreachables++;
						head_type[h] |= UNREACHABLE;
						sprintf (string, "%s is not reachable from the goal symbol. To ignore, rename to [%s].\n", head_name[h], head_name[h]);
						prt_error (string, head_name[h], 0, head_line[h]); 
					} 
				}
         }  
      }
      FREE (head_used, n_heads);

	// If we have any unreachables ...
		if (n_ignores && n_errors == 0)
		{
			if (n_prods + n_unreachables >= max_prods)
				MemCrash ("Number of productions", max_prods); 
			if (n_tails + n_unreachables >= max_tails)
				MemCrash ("Number of tail symbols", max_tails);
	  		RenumberProductions (n_ignores);
		}
}

///////////////////////////////////////////////////////////////////////////////

void  LG_CheckGrammar::RenumberProductions (int n_ignores)
{
		int h, p, t;
		int new_prod = l_prod[1];
		int new_tail = f_tail[l_prod[1]];
		for (t = n_tails-1; t >= new_tail; t--)
		{
			tail[t + n_ignores] = tail[t]; // Move nu places. 
		}
		n_tails += n_ignores;

  		for (p = n_prods-1; p >= f_prod[2]; p--)
		{
			f_tail   [p+n_ignores] = f_tail[p] + n_ignores; 
			l_tail   [p+n_ignores] = l_tail[p] + n_ignores; 
			ret_numb [p+n_ignores] = ret_numb[p];
			prod_line[p+n_ignores] = prod_line[p];
			prod_type[p+n_ignores] = prod_type[p];
		}								  
		n_prods += n_ignores;

		l_prod[1] += n_ignores;
  		for (h = 2; h < n_heads; h++)
		{
			f_prod[h] += n_ignores; 
			l_prod[h] += n_ignores; 
		}  
  
      for (h = 0; h < n_heads; h++)
      {
         if (head_type[h] & IGNORESYM)					
         {
			  	f_tail   [new_prod]   = new_tail;
			  	l_tail   [new_prod]   = new_tail+1;
				tail     [new_tail++] = -h;
			//	ret_numb [new_prod]   = -1;				// Ignore this symbol.
			  	ret_numb [new_prod]   = 2147483647;		// Ignore this symbol.
				ret_name [new_prod]   = "UNDEFINED";   // Assign name to defined constant.
				prod_line[new_prod]   = 0; 
				prod_type[new_prod++] = 0;
         }  
      }
    
		max_n_prods = n_prods;
      REALLOC (f_tail,		max_prods, n_prods);
      REALLOC (l_tail,		max_prods, n_prods);
      REALLOC (tail,			max_tails, n_tails);   
      REALLOC (ret_numb,   max_prods,  n_prods);
      REALLOC (prod_line,  max_prods,  n_prods);
      REALLOC (prod_type,  max_prods,  n_prods);
      REALLOC (ret_numb,   max_prods,  n_prods);
      REALLOC (prod_line,  max_prods,  n_prods);
      REALLOC (prod_type,  max_prods,  n_prods);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::P_UNREDUCIBLES ()
{
      char *red;
      int  h, p, t, s, n, m;

      ALLOC (red, n_heads);
      memset (red, 2, n_heads); /* undefined */
      m = 0;
      do 
      {
         n = 0;
         for (h = 0; h < n_heads; h++)
         {
            if (red [h] == 2)
            {
               for (p = f_prod [h]; p < l_prod [h]; p++)
               {
                  for (t = f_tail [p]; t < l_tail [p]; t++)
                  {
                     if ((s = tail [t]) < 0)
                     {
                        if (red [-s] != 1) goto nextp;
                     }  
                  }
                  n++;
                  red [h] = 1;
                  goto nexth;
nextp:            continue;
               }  
            }
nexth:      continue;
         }
         m += n;
      }
      while (n > 0);

      if (m < n_heads)
      {
         for (h = 0; h < n_heads; h++)
         {
            if (red [h] != 1)
            {
					prt_error ("Nonterminal symbol '%s' is in a cycle, cannot derive anything", head_name[h], 0, head_line[h]);
            }  
         }  
      }
      FREE (red, n_heads);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

int	LG_CheckGrammar::p_sym (int s, char *sp)
{
      char *p;

      if (s >= 0)                 /* Terminal symbol? */
         p = term_name[s];
      else                        /* Nonterminal symbol? */
         p = head_name[-s];

      prt_log ("%s%s", p, sp);
      return ((int)strlen(p) + (int)strlen(sp));
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::p_prod (int p, int dot, char *before)
{
      int t, u, d;
		if (p < 0)
		{
			p = -p;
	      prt_log ("%s%s -> ", before, head_name [head_sym [p]]);
		}
		else
		{
			prt_log ("%s%5d %s -> ", before, p, head_name [head_sym [p]]);
		}
      t = f_tail [p];
      u = l_tail [p];
      d = t + dot;
      if (dot == -1) d = u;
      for (;;)
      {
         if (t == d) prt_sta (". ");
         if (t >= u) break;
         p_sym (tail [t], " ");
         t++;
      }
      prt_log ("\n");
}

////////////////////////////////////////////////////////////////////////////////
//
//

char*	onstack;

void  LG_CheckGrammar::C_CYCLES ()
{		
		int h;
		ALLOC (onstack, n_heads);
		for (h = 0; h < n_heads; h++) onstack[h] = 0;
		C_CYCLES2(0);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

int   LG_CheckGrammar::C_CYCLES2 (int h)
{
		int p, t, nt;
		onstack[h] = 1;
		for (p = f_prod[h]; p < l_prod[h]; p++)
		{
			for (t = f_tail[p]; t < l_tail[p]; t++)
			{
				nt = -tail[t];
				if (nt > 0) // Nonterminal ?
				{
					if (t > f_tail[p])	// Not first tail symbol?
					{
						if (onstack[nt])	// Is it already on the stack?
						{
							prt_error ("'%s' is involved in an infinite cycle", head_name[nt], 0, head_line[nt]);
							return (0);
						}
						if (C_CYCLES2(nt) == 0) return (0);
					}
					else  // First tail symbol.
					{
						if (onstack[nt])	// Is it already on the stack?
						{
							if (nt != h)
							{
								prt_error ("'%s' is involved in an infinite cycle", head_name[nt], 0, head_line[nt]);
								return (0);
							}	 
						}
						else if (C_CYCLES2(nt) == 0) return (0);
					}
				}
			}
		}
		onstack[h] = 0;
		return (1);
}

////////////////////////////////////////////////////////////////////////////////
//
//
		int*   F_PROD;
		int*   L_PROD;
		int*   F_TAIL;
		int*   L_TAIL;
		int*   TAILSYM;
      int*   RET_NUMB;
      char** RET_NAME;
      char** HEAD_START;
		int    N_PRODS;
		int    N_TAILS;
		int    N_HEADS;
		char   vector [MAXPRODLENG]; 

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_CheckGrammar::DO_BACK_SUB () // Do back substitutions for nullable nonterminals. 
{
		int *HEAD, h, p, nn, T;

Top:  nn = 0;
		for (h = 0; h < n_heads; h++)
		{
			nullable[h] = 0;
			p = f_prod[h];
			if (l_tail[p] - f_tail[p] == 0) // length of first production is zero?
			{
				nn++;
				nullable[h] = 1;
			}
		}
		if (nn == 0) return;

	//	printf ("\n%d nullables ...\n", nn);

   	ALLOC (HEAD,       n_heads);
		ALLOC (HEAD_START, n_heads);
		ALLOC (F_PROD,     n_heads);
		ALLOC (L_PROD,     n_heads);
		ALLOC (F_TAIL,     max_prods);
		ALLOC (L_TAIL,     max_prods);
		ALLOC (TAILSYM,       max_tails);
		ALLOC (RET_NUMB,   max_prods);
		ALLOC (RET_NAME,   max_prods);

      N_HEADS = 0;
		N_PRODS = 0;
		N_TAILS = 0;
		memset (vector, 1, MAXPRODLENG);

      for (h = 0; h < n_heads; h++)
		{
			if (!nullable[h])
			{
				HEAD [h] = N_HEADS;
				F_PROD [N_HEADS] = N_PRODS;
				HEAD_START [N_HEADS] = head_name[h];
			//	printf ("%s ->\n", head_name[h]);
				for (p = f_prod[h]; p < l_prod[h]; p++)
				{
					scan_prod (h, p, f_tail[p]);
				}						 
				L_PROD [N_HEADS] = N_PRODS;
				N_HEADS++;
			}
			else if (f_tail [l_prod[h]] - f_tail [f_prod[h]] > 1) // More than one tail?
			{
				HEAD [h] = N_HEADS;
				F_PROD [N_HEADS] = N_PRODS;
				HEAD_START [N_HEADS] = head_name[h];
			//	printf ("%s ->\n", head_name[h]);
				for (p = f_prod[h]+1; p < l_prod[h]; p++) // Skip the null production.
				{
					make_prod (h, p);
				}						 
				L_PROD [N_HEADS] = N_PRODS;
				N_HEADS++;
			}
		}
  
		for (T = 0; T < N_TAILS; T++)
		{
			if (TAILSYM[T] < 0) TAILSYM[T] = -HEAD[-TAILSYM[T]];
		}

		FREE (HEAD,        n_heads);
		FREE (f_prod,      n_heads);
		FREE (l_prod,      n_heads);
		FREE (f_tail,      n_prods);
		FREE (l_tail,      n_prods);
		FREE (tail,        n_tails);
		FREE (ret_numb,    n_prods);
		FREE (ret_name,    n_prods);
		FREE (head_name,   n_heads);

		REALLOC (F_PROD,   n_heads,   N_HEADS);
		REALLOC (L_PROD,   n_heads,   N_HEADS);
		REALLOC (F_TAIL,   max_prods, N_PRODS);
		REALLOC (L_TAIL,   max_prods, N_PRODS);
		REALLOC (TAILSYM,     max_tails, N_TAILS);
		REALLOC (RET_NUMB, max_prods, N_PRODS);
		REALLOC (RET_NAME, max_prods, N_PRODS);

		f_prod     = F_PROD;
		l_prod     = L_PROD;
		f_tail     = F_TAIL;
		l_tail     = L_TAIL;
		tail       = TAILSYM;
		ret_numb   = RET_NUMB;
		ret_name   = RET_NAME;
		head_name = HEAD_START;

		if (N_PRODS > max_n_prods) max_n_prods = N_PRODS;

		n_tails    = N_TAILS;
		n_prods    = N_PRODS;
		n_heads    = N_HEADS;

//		PrintGrammar();
		goto Top;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void 	LG_CheckGrammar::scan_prod (int h, int p, int t) 
{
		int t0, sym;
		if (t < l_tail[p])
		{
			t0 = f_tail[p];
			if (vector [t-t0] == 1)
			{
				sym = tail[t];
				if (sym < 0 && nullable [-sym]) 
				{
					vector [t-t0] = 0;
					scan_prod (h, p, t+1);
					vector [t-t0] = 1;
				}
				scan_prod (h, p, t+1);
			}
		}
		else
		{
			make_prod (h, p);
		}
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void 	LG_CheckGrammar::make_prod (int h, int p) 
{
		int t, t0, tn, sym, q, n, P, I, T;

		F_TAIL   [N_PRODS] = N_TAILS;
		RET_NUMB [N_PRODS] = ret_numb[p];
		RET_NAME [N_PRODS] = ret_name[p];
	//	printf ("\nmake_prod (%d, %d) ...\n", h, p);

		n  = 0;
		t0 = f_tail[p];
		tn = l_tail[p];
		for (t = t0; t < tn; t++)
		{
			if (vector [t-t0] == 1) // keep this tail ?
			{
				n++;
				sym = tail[t];
Check:		if (sym < 0 && nullable [-sym])
				{
					q = f_prod[-sym];
					if (l_tail[q] - f_tail[q] == 0)           // Null first production?
					{
						if (l_tail [q+1] - f_tail [q+1] == 1)  // Only one tail on 2nd production?
						{
							sym = tail [f_tail[q+1]];           // First (and only) tail of 2nd prod.
							goto Check;	
						}
					}
				}
				if (N_TAILS >= max_tails) MemCrash ("Number of tail symbols", max_tails);
				TAILSYM[N_TAILS++] = sym;
			//	if (sym < 0) printf (" %s", head_name[-sym]);
			//	else         printf (" %s", term_name[ sym]);
			}
		}
	//	if (n == 0) printf ("<null>\n");
	//	else        printf ("\n");
		L_TAIL [N_PRODS] = N_TAILS;

	// Check for previous existence of this production. 
		if (n > 0)
		{
			if (n == 1 && -sym == h) goto Undo;
			for (P = F_PROD [N_HEADS]; P < N_PRODS; P++)
			{
				int PLO = L_TAIL [P]       - F_TAIL [P];
				int PLN = L_TAIL [N_PRODS] - F_TAIL [N_PRODS];
				if (PLO == PLN)
				{
					I = F_TAIL [N_PRODS];
					for (T = F_TAIL [P]; T < L_TAIL [P]; T++)
					{	
					//	char a[50]; 
					//	char b[50]; 
					//	symstr(TAILSYM[T], a);
					//	symstr(TAILSYM[I], b);
					//	printf ("Comparing %s to %s\n", b, a);
						if (TAILSYM[T] != TAILSYM[I++]) goto NextP;
					}
				//	printf ("already exists\n\n");
					goto Undo; // Production already exists!
				}
NextP:		continue;
			}
		//	printf ("new production accepted\n");
		}
		else // New production is NULL!
		{
			int FP = F_PROD [N_HEADS];
			if (N_PRODS > FP) // Already have some productions?
			{
				if (F_TAIL [FP] == L_TAIL [FP]) goto Undo; // Already have NULL.
			}
		// Move current productions down one. 
			for (P = N_PRODS; P > FP; P--)
			{
				F_TAIL [P] = F_TAIL [P-1];
				L_TAIL [P] = L_TAIL [P-1];
			}
		// Create NULL production in first place. 
			F_TAIL [P] = F_TAIL [P+1];
			L_TAIL [P] = F_TAIL [P+1];
		//	printf ("<null> production accepted\n");
		}
		if (++N_PRODS >= max_prods) 
			MemCrash ("Number of productions", max_prods);
      return;

Undo: N_TAILS = F_TAIL [N_PRODS]; // Reset number of tails.
	//	printf ("new production rejected\n");
		return;
}

/*--- End of Chk_Gram.C ----------------------------------------------------*/

