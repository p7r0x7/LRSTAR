
/* Copyright 2018 Paul B Mann.  BSD License. */

      #include "CM_Global.h"
		#include "LG_Global.h"
		#include "LG_BuildStates.h"

      int    LG_BuildStates::N_states;
		int	 LG_BuildStates::extra_states;
		int*   LG_BuildStates::ntt_item;
		int*   LG_BuildStates::accessor;
		int    LG_BuildStates::accept_state; 
		char** LG_BuildStates::FIRST;
		char** LG_BuildStates::FOLLOW;
		int*   LG_BuildStates::f_kernel;
		int*   LG_BuildStates::l_kernel;
		int*   LG_BuildStates::f_final;
		int*   LG_BuildStates::l_final; 
		int*   LG_BuildStates::final;
		int*   LG_BuildStates::kernel;
		ITEM*  LG_BuildStates::item;    
		int*   LG_BuildStates::f_item;
		int    LG_BuildStates::n_ttran;
		int    LG_BuildStates::n_nttran;
		int*   LG_BuildStates::ntt_origin;
		int    LG_BuildStates::n_kernels;
		int    LG_BuildStates::n_finals;
		int    LG_BuildStates::org_states;
		int    LG_BuildStates::n_items;
		int*   LG_BuildStates::ntt_start;
		int*   LG_BuildStates::ntt_end;
		int*   LG_BuildStates::ntt_symb;
		int    LG_BuildStates::ntt_states;
		int*   LG_BuildStates::ntt_action;
		int    LG_BuildStates::n_ttas;
		int*   LG_BuildStates::tt_start;
		int*   LG_BuildStates::tt_end;
		int*   LG_BuildStates::tt_action;
		int*   LG_BuildStates::tt_symb;
		int    LG_BuildStates::tt_states;
		int*   LG_BuildStates::f_camefrom;
		int*   LG_BuildStates::l_camefrom;
		int*   LG_BuildStates::camefrom;

      static int    n_clo;
		static uint   hash_div;
		static uint   hash_no;
		static int    head_free;
      static int    term_free;
		static int    n_camefroms;
		static int    accept_tran;

		static int*   added;
		static int*   already;
		static SYMLIST* slist;
		static CLOSURE* closure;
		static int*   Tcameto;
		static int*   Ncameto;


///////////////////////////////////////////////////////////////////////////////
//                                                                           //

int   LG_BuildStates::BuildStates () /* Build LR0 States */
{
      int p, state, t;

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Building states ...\n");

		max_states  = optn[MAX_STA];
      max_final   = optn[MAX_FIN]; 
      max_kernel  = optn[MAX_KER]; 
      max_ntt     = optn[MAX_NTT]; 
      max_tt      = optn[MAX_TT ]; 
      max_child   = optn[MAX_CH ]; 

      ALLOC (Tcameto,   max_states+1);
      ALLOC (Ncameto,   max_states+1);
      ALLOC (st_type,   max_states);
      ALLOC (accessor,  max_states);
      ALLOC (f_final,   max_states+2);
      ALLOC (f_kernel,  max_states+2);
      ALLOC (tt_start,  max_states+1);
      ALLOC (ntt_start, max_states+1);

      ALLOC (final,     max_final);
      ALLOC (kernel,    max_kernel);
      ALLOC (tt_action,   max_tt);
      ALLOC (ntt_action,  max_ntt);

      ALLOC (added,     n_heads);
      ALLOC (slist,     n_symbs+1);
      ALLOC (closure,   2*n_prods+1);
      ALLOC (already,   n_symbs+1);

      FASTINI (-1, added,      n_heads);
      FASTINI ( 0, Tcameto,    max_states+1);
      FASTINI ( 0, Ncameto,    max_states+1);

      n_finals      = 0;
      n_kernels     = 0;
      f_final [0]   = 0;
      f_kernel [0]  = 0;
      accessor [0]  = 0;

		C_ITEMS ();

      for (p = f_prod [0]; p < l_prod [0]; p++)
      {
         if (prod_len [p] > 0)
              kernel [n_kernels++] = f_item [p];
         else final [n_finals++] = p;
      }

      added [0]  = 0;
      n_nonttran = 0;
      n_termtran = 0;
      f_kernel [1] = n_kernels;

		if (optn[LG_VERBOSE] > 2)
			printf ("Starting state expansions ...\n");

   // Create non-reduce-only states, except for "$start -> Token . $end" state.
      N_states = 1;
      for (state = 0; state < N_states; state++)
      {
         EXPAND (state);
      }

	// Create Accept & Final state.
		MAKE_ACCEPT_STATE ();
      MTSL ();

	// Define accept state.
		for (t = ntt_start[0]; t < ntt_start[1]; t++)
		{
			if (ntt_symb[t] == 1) // Token transition?
			{
				accept_state = ntt_action[t];
				break;
			}
		}

		org_states = N_states;		
      if (optn[LG_VERBOSE])
		     prt_log     ("States   %7d states before converting to a DFA.\n", N_states);
		else prt_logonly ("States   %7d states before converting to a DFA.\n", N_states);

	   if (optn[LG_VERBOSE] > 2)
      prt_log ("Trans    %7d T transitions, %d NT transitions.\n", n_termtran, n_nonttran);

      FREE (f_item,   n_prods+1);
      FREE (already,  n_symbs+1);
      FREE (closure,  2*n_prods+1);
      FREE (slist,    n_symbs+1);
      FREE (added,    n_heads);

      REALLOC (ntt_action,  max_ntt,  n_nonttran);	
      REALLOC (tt_action,   max_tt,   n_termtran); 
      REALLOC (kernel,    max_kernel,   n_kernels );
      REALLOC (final,     max_final,    n_finals  );
      REALLOC (ntt_start, max_states+1, N_states+2);
      REALLOC (tt_start,  max_states+1, N_states+2);
      REALLOC (f_kernel,  max_states+2, N_states+2);
      REALLOC (f_final,   max_states+2, N_states+2);
      REALLOC (accessor,  max_states,   N_states  );
      REALLOC (st_type,   max_states,   N_states  );

		C_CAMEFROM (N_states, tt_start, tt_action, ntt_start, ntt_action, f_camefrom, camefrom); 

      n_oprods = n_prods;  // Set number of original productions. 

	  	if (optn[LG_VERBOSE] > 2)
	  	printf ("Done with building states !!!\n");
      return (1);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_BuildStates::C_ITEMS ()
{
      int h, i, p, d, t, np;

      i = 0;
		np = 0;
      for (h = 0; h < n_heads; h++)
      {
         for (p = f_prod[h]; p < l_prod[h]; p++)
         {
				np++;
            for (t = f_tail[p]; t < l_tail[p]; t++)
            {
               i++;
            }
            i++;
         }  
      }

      n_items = i;
      ALLOC (head_sym,  n_prods);
      ALLOC (prod_len,  n_prods);
      ALLOC (f_item,    n_prods+1);
      ALLOC (item,      n_items);

      i = 0;
      for (h = 0; h < n_heads; h++)
      {
         for (p = f_prod[h]; p < l_prod[h]; p++)
         {
            d = 0;
            f_item [p] = i;
            head_sym [p] = h;
            for (t = f_tail[p]; t < l_tail[p]; t++)
            {
               item[i].symb  = tail [t];
               item[i].prod  = p;
               item[i++].dot = d++;
            }
            prod_len[p]    = d;
            item[i].prod   = p;
            item[i].dot    = d;
            item[i++].symb = -32767;
         }  
      }
      if (i != n_items) InternalError(4);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_BuildStates::EXPAND (int state)
{
      int i, n, w, tt=0, nt=0, nfi;
      DO_CLOSURE (state);
      tt_start  [state] = n_termtran;
      ntt_start [state] = n_nonttran;
      if (n_clo)
      {
         for (i = n_heads; i < term_free; i++)
         {
				tt++;
				n = n_kernels;
				MAKE_KERNEL (slist[i].start);
				w = TRANSIT (n, slist[i].symb);
				if (w > 0) Tcameto [w]++;
				tt_action [n_termtran++] = w;
				if (n_termtran >= max_tt) 
				MemCrash ("Number of terminal transitions", max_tt);
         }
         for (i = 0; i < head_free; i++)	
         {
				nt++;
				if (slist[i].symb == 1) // if "Token" transition
				{
					accept_tran = n_nonttran; // capture this for later.	
					ntt_action [n_nonttran++] = 0;
				}
				else
				{
					n = n_kernels;
					MAKE_KERNEL (slist[i].start);
					w = TRANSIT (n, (int)-slist[i].symb);
					if (w > 0) Ncameto [w]++;
					ntt_action [n_nonttran++] = w;
				}
				if (n_nonttran >= max_ntt) 
				MemCrash ("Number of nonterminal transitions", max_ntt);
         }  
      }
      tt_start  [state+1] = n_termtran;
      ntt_start [state+1] = n_nonttran;

   // Define state type for this state based on transition data.
      st_type[state] = 0;
      if (tt) st_type[state] |= TT_STATE; 
      if (nt) st_type[state] |= NT_STATE; 
      if (tt == 0 && nt == 0) 
      { 
         nfi = f_final[state+1] - f_final[state];
         if (nfi > 1) st_type[state] = MR_STATE; 
         else         st_type[state] = RO_STATE; 
      }
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_BuildStates::DO_CLOSURE (int state)
{
      int k;
      n_clo = 0;
      head_free = 0; 
      term_free = n_heads;
      FASTINI (-1, already, n_symbs+1);

      for (k = f_kernel [state]; k < f_kernel [state+1]; k++)
      {
         ADD_ITEM (kernel [k], state);
      }
      f_final [state+1] = n_finals;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_BuildStates::ADD_ITEM (int i, int state)
{
      int s, symb_i, symb_free, p;
      if ((s = item[i].symb) != -32767) /* IF NOT FINAL */
      {
         closure [n_clo].item = i;
         closure [n_clo].next = -1;
         if (already [s+n_heads] == -1) /* New symbol. */
         {
            if (s < 0)
            {
               slist [symb_free = head_free++].symb = -s;
               slist [symb_free].start = n_clo;
               slist [symb_free].end = n_clo;
               already [s+n_heads] = symb_free;
               n_clo++;
               if (added [-s] != state)
               {
                  for (p = f_prod [-s]; p < l_prod [-s]; p++)
                  {
                     ADD_ITEM (f_item [p], state);
                  }
                  added [-s] = state;
               }  
            }
            else
            {
               slist [symb_free = term_free++].symb = s;
               slist [symb_free].start = n_clo;
               slist [symb_free].end = n_clo;
               already [s+n_heads] = symb_free;
               n_clo++;
            }  
         }
         else
         {
            symb_i = already [s+n_heads];
            closure [slist [symb_i].end].next = n_clo;
            slist [symb_i].end = n_clo;
            n_clo++;
         }
      }
      else
      {
         if (n_finals >= max_final) MemCrash ("Number of final items", max_final);
         final [n_finals++] = item[i].prod;
      }
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_BuildStates::MAKE_KERNEL (int c)
{
      int i;
      hash_no = 0;
      do
      {
         i = closure [c].item + 1;
         hash_no += kernel [n_kernels++] = i;
         if (n_kernels >= max_kernel) 
				MemCrash ("Number of kernel items", max_kernel);
      }
      while ((c = closure [c].next) != -1);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

int	LG_BuildStates::TRANSIT (int ki, int sym)
{
      int ni;
      ni = n_kernels - ki;
      if (ni > 1)                     // Number of items is 1?
      {
		// Remove this SORT and save 1/2 sec on English.lgr, 
		// but better to have sorted items in the state listing. 
			SORT (kernel + ki, kernel + n_kernels);	
		}

      if (N_states >= max_states) 
			MemCrash ("Number of states", max_states);
      accessor [N_states]   = sym;
      f_kernel [N_states+1] = n_kernels;
	//	if (((N_states+1) %  10000) == 0) printf ("*");
	//	if (((N_states+1) % 800000) == 0) printf ("\n");
      return (N_states++);
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void	LG_BuildStates::MAKE_ACCEPT_STATE ()
{
	// Make accept_state
      if (N_states >= max_states) 
			MemCrash ("Number of states", max_states);
      if (n_kernels >= max_kernel) 
			MemCrash ("Number of kernel items", max_kernel);
		if (n_termtran >= max_tt) 
			MemCrash ("Number of terminal transitions", max_tt);

		accept_state = N_states;
		ntt_action [accept_tran]  = accept_state;	// Token transition to accept state.
		Ncameto  [accept_state]++;						// Came to accept_state.
		f_kernel [accept_state] = n_kernels;	 
		f_final  [accept_state] = n_finals;
      accessor [accept_state] = -1;					// Token
		st_type  [accept_state] = TT_STATE;
		tt_start [accept_state] = n_termtran;
		ntt_start[accept_state] = n_nonttran;
      kernel   [n_kernels]    = f_item [0] + 1;	// $start -> Token . $end
		tt_action  [n_termtran]   = N_states+1;
		n_termtran++;
      n_kernels++;
      N_states++;

	// Make final_state
      if (N_states >= max_states) 
			MemCrash ("Number of states", max_states);
      if (n_finals  >= max_final ) 
			MemCrash ("Number of final items", max_final);
      if (n_kernels >= max_kernel) 
			MemCrash ("Number of kernel items", max_kernel);

		final_state = N_states;
		Tcameto  [final_state]++;					
		f_kernel [final_state]  = n_kernels;
		f_final  [final_state]  = n_finals;
		tt_start [final_state]  = n_termtran;
		ntt_start[final_state]  = n_nonttran;
      st_type  [final_state]  = RO_STATE;
      accessor [final_state]  = tail [l_tail [0] - 1]; 
      tt_start [final_state+1] = n_termtran;
      ntt_start[final_state+1] = n_nonttran;
      final [n_finals ] = 0; // Goal production.
      kernel[n_kernels] = f_item [0] + prod_len [0];
      n_finals++;
      n_kernels++;
      N_states++;

      f_final [N_states] = n_finals;
      f_kernel[N_states] = n_kernels;
}

///////////////////////////////////////////////////////////////////////////////
//                                                                           //

void  LG_BuildStates::MTSL ()  // Make Transition Symbols List.
{
      int i;

      ALLOC (tt_symb,  n_termtran);
      ALLOC (ntt_symb, n_nonttran);

      for (i = 0; i < n_termtran; i++)
      {
         if (tt_action[i] > 0) tt_symb[i] = accessor [tt_action [i]];
         else					    tt_symb[i] = tail[l_tail[-tt_action[i]]-1];
      }

      for (i = 0; i < n_nonttran; i++)
      {
         if (ntt_action[i] > 0) ntt_symb[i] = -accessor [ntt_action [i]];
         else						  ntt_symb[i] = -tail[l_tail[-ntt_action[i]]-1];
      }
}

//
///////////////////////////////////////////////////////////////////////////////


