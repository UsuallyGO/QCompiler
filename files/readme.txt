
Files in files/ directory are resource files for test.

grammar.syn 
'#' stands for epsilon
'$' stands for the end of the input files
=====================================================
For grammar:
<A>-><B><C>c|g<D><B>
<B>->b<C><D><E>|#
<C>-><D>a<B>|c a
<D>->d<D>|#
<E>->g<A>f|c

Term       First Set        Follow Set
  A     {a, b, c, d, g}	  {f, $}
  B	    {b, #}	          {a, c, d, g, f, $}
  C	    {a, c, d}	      {c, d, g}
  D	    {d, #}	          {a, b, c, g, f, $}
  E	    {c, g}	          {a, c, d, g, f, $}

Select set:
Select(<A>-><B><C>c) = {a, b, c, d}
Select(<A>->g<D><B>) = {g}
Select(<B>->b<C><D><E>) = {b}
Select(<B>->#) = {a, c, d, g, f, $}
Select(<C>-><D>a<B>) = {a, d}
Select(<C>->ca) = {c}
Select(<D>->d<D>) = {d}
Select(<D>->#) = {a, b, c, g, f, #}
Select(<E>->g<A>f) = {g}
Select(<E>->c) = {c}
  
=====================================================
For grammar:
<S>-><A><B>c
<A>->a|#
<B>->b|#

Term      First Set         Follow Set
 A         {a, #}            {b, c}        
 B         {b, #}            {c}
 S         {a,b,c}           {$}
 
=====================================================
 For grammar:
 <S>-><A><B>|b<C>
 <A>->b|#
 <B>->a<D>|#
 <C>-><A><D>|b
 <D>->a<S>|c

 Term      First Set         Follow Set
   S       {a, b, #}            {$}
   A       {b, #}             {a,c,$}
   B       {a, #}               {$}
   C       {a, b, c}            {$}
   D       {a, c}               {$
=====================================================
For grammar:
<E>-><T><E1>
<E1>->+<T><E1>|#
<T>-><F><T1>
<T1>->*<F><T1>|#
<F>->(<E>)|id

 Term     First set         Follow Set
   E      {(, id}             {), $}
   E1     {+, #}              {), $}
   T      {(, id}             {+, ), $}
   T1     {*, #}              {+, ), $}
   F      {(, id}             {+, *, ), $}

=====================================================
For grammar:
<S>-><ZA>a|b
<ZA>-><ZA>c|<S>d|#

after eliminating left recursion
<S>-><ZA>a|b
<ZA>->bd<ZA1>|<ZA1>
<ZA1>->c<ZA1>|ad<ZA1>|#

=====================================================
For grammar:
<E>-><E>+<T>|<T>
<T>-><T>*<F>|<F>
<F>->(<E>)|id

after eliminating left recursion
<E>-><T><E1>
<E1>->+<T><E1>|#
<T>-><F><T1>
<T1>->*<F><T1>|#
<F>->(<E>)|id

=====================================================
For grammar:
<A>->a<B>
<A>-><B>b
<B>-><A>c
<B>->d

after eliminating left recursion
<A>->a<B>
<A>-><B>b
<B>->(a<B>c|d)<B1>
<B1>->bc<B1>|#

=====================================================
For grammar:
<S>-><Q>c|c
<Q>-><R>b|b
<R>-><S>a|a

after elimination left recursion
<S>->abc<S1>|bc<S1>|c<S1>
<S1>->abc<S1>|#

=====================================================
For grammar:
<S>-><S><X>|<S><S>b|<X><S>|a

after eliminating left recursion
<S>-><X><S><S1>|a<S1>
<S1>-><X><S1>|<S>b<S1>|#

=====================================================
For grammar:
<A>->a b1|a b2

after left factoring
<A>->a<A1>
<A1>->b1|b2

=====================================================
For grammar:
<S>->i<E>t<S>|i<E>t<S>e<S>|a
<E>->b

after left factoring
<S>->i<E>t<S><S1>|a
<S1>->e<S>|#
<E>->b

=====================================================
For grammar:
<A>-><B>a|<B>
<B>->b

after left factoring
<A>-><B><A1>
<B>->b
<A1>->a|#

=====================================================
For grammar:
<S>-><E>
<E>-><E>+<T>|<E>-<T>|<T>
<T>-><T>*<F>|<T>/<F>|<F>
<F>->id|num|(<E>)

after eliminating left recursion:
<S>-><E>
<E>-><T><E1>
<E1>->+<T><E1>
<E1>->-<T><E1>
<E1>->#
<T>-><F><T1>
<T1>->*<F><T1>
<T1>->/<F><T1>
<T1>->#
<F>->id
<F>->num
<F>_>(<E>)

     nullable      First        Follow
 S     no         (, id, num
 E     no         (, id, num      ), $
 E1    yes          +, -          ), $
 T     no         (, id, num     ), +, -, $
 T1    yes          *, /         ), +, -, $
 F     no         (, id, num     ), *, /, +, -, $

 Predictive parsing table(columns for num, /, and -, are omiited)
 
       +       *        id       (        )        $
 S                     S->E$    S->E$
 E                     E->TE1   E->TE1
 E1  E1->+TE1                           E1->#     E1->#
 T                     T->FT1   T->FT1
 T1  T1->   T1->*FT1                    T1->#     T1->#
 F                     F->id    F->(E)