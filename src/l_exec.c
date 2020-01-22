/*
 *	Lilith
 *
 *	siehe : Appendix 1 of the yellow report :
 *		N.Wirth. The Personal Computer Lilith.
 *		Institut fuer Informatik, ETH. Report 40, 1981.
 */

#include	<stdio.h>
#include	<assert.h>
#include	"l.h"

extern	int	no_checks;

word	*stack;
char	*code;
int	datasize;
int	stacksize;
int	codesize;

word	PC;	/* program counter */
word	IR;	/* instruction register */
word	F;	/* code frame base address */
word	G;	/* data frame base address */
word	H;	/* stack limit address */
word	L;	/* local segment address */
word	S;	/* stack pointer */
word	P;	/* process base address */
word	M;	/* process interrupt mask */
word	REQ;	/* interrupt request */
word	ReqNo;	/* request number, 8..15 */

int	bootflag = TRUE;
#ifdef TRACE
int	act_mod; /* aktuelles Modul */
int	may_print;
extern	char	*mnem[];
#define RANGE TRUE
#undef LT
#endif

#ifdef LT
int	may_print;
extern	char	*mnem[];
word	rge_low,rge_high;
int	act_mod;
int	aux;
#define RANGE (F*4+PC>=rge_low && F*4+PC<=rge_high)
# define TRACE
#endif

word
next ()
{
	++PC;
#ifdef TRACE
	if ( may_print && RANGE )
		trace(" %3o",code [ 4*F+PC-1 ]);
#endif
	return (word) code [ 4*F+PC-1 ];
}

short
snext ()
{
	short	val;

	++PC;
	val = (short) code [ 4*F+PC-1 ] & 0x7F;
	if ( code [ 4*F+PC-1 ] & 0x80 ) val |= 0xFF80;
#ifdef TRACE
	if ( may_print && RANGE )
		trace(" %3o",val);
#endif
	return val;
}

word
next2 ()
{
	PC += 2;
#ifdef TRACE
	if ( RANGE )
		trace(" %5o",code [ 4*F+PC-2 ]*0x100 + code [ 4*F+PC-1 ]);
#endif
	return code [ 4*F+PC-2 ]*0x100 + code [ 4*F+PC-1 ];
}

/*
 *	ExpressionStack
 */

static	int	sp = 0;
word	a[16];

#ifdef TRACE
showstack()
{
	int	index;

	if ( RANGE )
		for ( index = 0 ; index < sp ; ++index )
			trace("a[%2d] = %5o\n",index,a[index]);
}
#endif TRACE

push ( x )
word	x;
{
#ifdef TRACE
	if ( sp >= 16 )
		crash("Expression Stack Overflow");
#endif TRACE
	a[sp++] = x;
}

word
pop ()
{
#ifdef TRACE
	if ( empty() )
		crash("Expression Stack Underflow");
#endif TRACE
	return a[--sp];
}

dpush ( d )
long	d;
{
	union {
	long	f;
	word	w[2];
	} x;

	x.f = d;
	push ( x.w[0] );
	push ( x.w[1] );
}

long
dpop ()
{
	union {
	long	f;
	word	w[2];
	} x;

	x.w[1] = pop();
	x.w[2] = pop();
	return x.f;
}

empty ()
{
	return sp == 0;
}

mark ( x , external )
word	x;
int	external;
{
	word	i;

	i = S;
	stack[S++] = x;		/* static link */
	stack[S++] = L;		/* dynamic link */
	if ( external )
		stack[S] = PC | 0x8000;
	else
		stack[S] = PC;
	S += 2;
	L = i;
}

saveexpstack ()
{
	word	c;

	c = 0;
	while ( ! empty() ) {
		stack[S++] = pop();
		++c;
		}
	stack[S++] = c;
}

restoreexpstack ()
{
	word	c;

	c = stack[--S];
	while ( c > 0 ) {
		--c;
		push ( stack[--S] );
		}
}

saveregs ()
{
	saveexpstack();
	stack[P  ] = G;	stack[P+1] = L;
	stack[P+2] = PC;stack[P+3] = M;
	stack[P+4] = S; stack[P+5] = H+24;
}

restoreregs(changemask)
int	changemask;
{
	G = stack[P];	F = stack[G];
	L = stack[P+1];
	if ( ! bootflag )
		PC = stack[P+2];
	else {
		PC = 0;
		PC = next2();		/* proc # 0 wird zuerst aufgerufen */
		stack[P+2] = PC;
		}
	if ( changemask )
		M = stack[P+3];
	S = stack[P+4];	H = stack[P+5] - 24;
	if ( ! bootflag )
		restoreexpstack();
}

transfer(changemask,to,from)
int	changemask;
word	to,from;
{
	word	j;

#ifdef TRACE
	trace("transfer: to = %o, from = %o\n",to,from);
#endif
	j = stack[to];
	saveregs();
	stack[from] = P;
	P = j;
	restoreregs(changemask);
}

trap(n)
word	n;
{
	crash("\n\nTRAP %o\n\n",n);
	if ( ! (0x0001<<n & stack[P+7]) ) {
		stack[P+6] = n;
		transfer(TRUE,TLC,TLC+1);
		}
}

execute ()
{
	register word	i,j,k;
	register short	si,sj,sk;
	register word	sz,adr;
		 short	low,hi;
		 short  ssz;
		 char	ch;
	register long	x,y;
	register float  fx,fy;

	P = stack[4];
	restoreregs(TRUE);
	REQ = FALSE;
	bootflag = FALSE;
#ifdef	TRACE
	trace("execute: lilith is ready for execution\n");
	act_mod = 1;
	trace("module # %3o, proc # %3o\n",act_mod,0);
#endif	TRACE
#ifdef	LT
	printf("rge_low = "); scanf("%o",&aux); rge_low = (word) aux; printf("%o\n",rge_low);
	printf("rge_high = "); scanf("%o",&aux); rge_high = (word) aux; printf("%o\n",rge_high);
	getchar(); /* '\n' abholen */
	trace("rge_low = %o, rge_high = %o\n",rge_low,rge_high);
#endif	LT

	do {
	    /* wird nicht benoetigt
	    if ( REQ )
		transfer(TRUE,2*ReqNo,2*ReqNo+1);
	    */
	    /* aus Schnelligkeitsgruenden gestrichen ...
	    if ( S >= datasize+stacksize )
		quit("stack segment overflow");
	    */
#ifdef  TRACE
	    may_print = FALSE;
#endif
	    IR = next();
#ifdef	TRACE
	    may_print = TRUE;
	    if ( RANGE )
		    trace("%5o %5o %5o %5o %-5s",4*F+PC-1,(PC-1)/2,PC-1,IR,mnem[IR]);
#endif
	    if ( IR < 020 ) /* LI0 - LI15 */
		push ( IR % 16 );
	    else if ( IR < 040 ) switch ( IR ) {
		case 020 : /* LIB */
			push(next()); break;
		case 022 : /* LIW */
			push(next2()); break;
		case 023 : /* LID */
			push(next2()); push(next2()); break;
		case 024 : /* LLA */
			push(L+next()); break;
		case 025 : /* LGA */
			push(G+next()); break;
		case 026 : /* LSA */
			push(pop()+next()); break;
		case 027 : /* LEA */
			i = next();
			j = next();
			push(stack[DFT+i]+j); break;
		case 030 : /* JPC */
			if ( ! pop() ) {
				i = next2();
				PC += i-2;
				}
			else
				PC += 2;
			break;
		case 031 : /* JP */
			i = next2();
			PC += i-2; break;
		case 032 : /* JPFC */
			if ( ! pop() )
				PC += next() -1;
			else
				++PC;
			break;
		case 033 : /* JPF */
			PC += next() -1; break;
		case 034 : /* JPBC */
			if ( ! pop() )
				PC -= next() +1;
			else
				++PC;
			break;
		case 035 : /* JPB */
			PC -= next() +1; break;
		case 036 : /* ORJP */
			if ( ! pop() )
				++PC;
			else {
				push(1);
				PC += next() -1;
				}
			break;
		case 037 : /* ANDJP */
			if ( ! pop() ) {
				push(0);
				PC += next() -1;
				}
			else
				++PC;
			break;
		}
	    else if ( IR < 044 ) switch ( IR ) {
		case 040 : /* LLW */
			push(stack[L+next()]); break;
		case 041 : /* LLD */
			i = L+next();
			push(stack[i]);
			push(stack[i+1]);
			break;
		case 042 : /* LEW */
			i = next();
			j = next();
			push(stack[stack[DFT+i]+j]); break;
		case 043 : /* LED */
			j = next();
			i = stack[DFT+j]+next();
			push(stack[i]);
			push(stack[i+1]);
			break;
		}
	    else if ( IR < 060 ) /* LLW4 - LLW15 */
		push(stack[L + IR % 16]);
	    else if ( IR < 064 ) switch ( IR ) {
		case 060 : /* SLW */
			stack[L+next()] = pop(); break;
		case 061 : /* SLD */
			i = L+next();
			stack[i+1] = pop();
			stack[i] = pop();
			break;
		case 062 : /* SEW */
			i = next();
			j = next();
			stack[stack[DFT+i]+j] = pop(); break;
		case 063 : /* SED */
			j = next();
			i = stack[DFT+j]+next();
			stack[i+1] = pop(); stack[i] = pop();
			break;
		}
	    else if ( IR < 0100 ) /* SLW4-SLW15 */
		stack[L+ IR % 16] = pop();
	    else if ( IR < 0102 ) switch ( IR ) {
		case 0100 : /* LGW */
			push(stack[G+next()]); break;
		case 0101 : /* LGD */
			i = next() + G;
			push(stack[i]);
			push(stack[i+1]);
			break;
		}
	    else if ( IR < 0120 ) /* LGW2 - LGW15 */
		push(stack[G+ IR % 16]);
	    else if ( IR < 0122 ) switch ( IR ) {
		case 0120 : /* SGW */
			stack[G+next()] = pop(); break;
		case 0121 : /* SGD */
			i = G+next();
			stack[i+1] = pop();
			stack[i] = pop();
			break;
		}
	    else if ( IR < 0140 ) /* SGW2 - SGW15 */
		stack[G + IR % 16] = pop();
	    else if ( IR < 0160 ) /* LSW0 - LSW15 */
		push(stack[pop() + IR % 16]);
	    else if ( IR < 0200 ) { /* SSW0 - SSW15 */
		k = pop();
		i = pop() + IR % 16;
		stack[i] = k;
		}
	    else if ( IR < 0240 ) switch ( IR ) {
		case 0200 : /* LSW */
			i = pop() + next();
			push(stack[i]);
			break;
		case 0201 : /* LSD */
			i = pop() + next();
			push(stack[i]);
			push(stack[i+1]);
			break;
		case 0202 : /* LSDO */
			i = pop();
			push ( stack[i] );
			push ( stack[i+1] );
			break;
		case 0203 : /* LXFW */
			k = pop() + pop() * 4;
			push(stack[k]);
			break;
		case 0204 : /* LSTA */
			push(stack[G+2]+next()); break;
		case 0205 : /* LXB */
			i = pop();
			j = pop();
			k = stack[ j + i/2 ];
			if ( i % 2 == 0 )
				push ( k / 0x100 );
			else
				push ( k % 0x100 );
			break;
		case 0206 : /* LXW */
			i = pop() + pop();
			push(stack[i]);
			break;
		case 0207 : /* LXD */
			i = 2*pop()+pop();
			push(stack[i]);
			push(stack[i+1]);
			break;
		case 0210 : /* DADD */
			y = dpop();
			x = dpop();
			dpush(x+y);
			break;
		case 0211 : /* DSUB */
			y = dpop();
			x = dpop();
			dpush(x-y);
			break;
		case 0212 : /* DMUL */
			j = pop();
			i = pop();
			x = (long) (i * j);
			dpush(x);
			break;
		case 0213 : /* DDIV */
			j = pop();
			x = dpop();
			k = ((word) x ) / j;
			i = ((word) x ) / j;
			push(i);
			push(k);
			break;
		case 0216 : /* DSHL */
			x = dpop();
			x <<= 1;
			dpush(x);
			break;
		case 0217 : /* DSHR */
			x = dpop();
			x >>= 1;
			dpush(x);
			break;
		case 0220 : /* SSW */
			k = pop();
			i = pop() + next();
			stack[i] = k;
			break;
		case 0221 : /* SSD */
			k = pop();
			j = pop();
			i = pop() + next();
			stack[i] = j;
			stack[i+1] = k;
			break;
		case 0222 : /* SSD0 */
			k = pop();
			j = pop();
			i = pop();
			stack[i] = j;
			stack[i+1] = k;
			break;
		case 0223 : /* SXFW */
			i = pop();
			k = pop() + pop()*4;
			stack[k] = i;
			break;
		case 0224 : /* TS */
			i = pop();
			push(stack[i]);
			stack[i] = 1;
			break;
		case 0225 : /* SXB */
			k = pop();
			i = pop();
			j = pop() + i / 2;
			if ( i % 2 == 0 )
				stack[j] = k * 0x100 + stack[j] % 0x100;
			else
				stack[j] = (stack[j]/0x100) * 0x100 + k;
			break;
		case 0226 : /* SXW */
			k = pop();
			i = pop() + pop();
			stack[i] = k;
			break;
		case 0227 : /* SXD */
			k = pop();
			j = pop();
			i = 2 * pop() + pop();
			stack[i] = j;
			stack[i+1] = k;
			break;
		case 0230 : /* FADD */
			fy = dpop();
			fx = dpop();
			dpush(fx+fy);
			break;
		case 0231 : /* FSUB */
			fy = dpop();
			fx = dpop();
			dpush(fx-fy);
			break;
		case 0232 : /* FMUL */
			fy = dpop();
			fx = dpop();
			dpush(fx*fy);
			break;
		case 0233 : /* FDIV */
			fy = dpop();
			fx = dpop();
			dpush(fx/fy);
			break;
		case 0234 : /* FCMP */
			fx = dpop();
			fy = dpop();
			push( fx < fy );
			push( fx > fy );
			break;
		case 0235 : /* FABS */
			fx = dpop();
			dpush( fx >= 0 ? fx : -fx);
			break;
		case 0236 : /* FNEG */
			dpush(-dpop()); break;
		case 0237 : /* FFCT */
			switch ( next() ) {
			case 0 : dpush( (float) pop() ); break;
			case 1 : dpush( (float) pop() ); break;
			case 2 : push ( (word) dpop() ); break;
			case 3 : crash("FFCT 3 not yet implemented"); break;
			default: crash("FFCT %1d: ill. argument"); break;
			}
			break;
		}
	    else if ( IR < 0300 ) switch ( IR ) {
		case 0240 : /* READ */
			crash("READ not implemented"); break;
		case 0241 : /* WRITE */
			crash("WRITE not implemented"); break;
		case 0242 : /* DSKR */
			crash("DSKR not implemented"); break;
		case 0243 : /* DSKW */
			crash("DSKW not implemented"); break;
		case 0244 : /* SETRK */
			crash("SETRK not implemented"); break;
		case 0245 : /* UCHK */
			k = pop();
			j = pop();
			i = pop();
			push(i);
			if (no_checks) break;
			if ( i < j || i > k )
				crash("UCHK fails: %o [%o..%o]\n",i,j,k);
			break;
		case 0246 : /* SVC - supervisor call */
			svc ( next() ); break;
		case 0247 : /* SYS */
			crash("SYS not implemented"); break;
		case 0250 : /* ENTP */
			stack[L+3] = M;
			M = 0xFFFF << (16-next());
			break;
		case 0251 : /* EXP */
			M = stack[L+3];
			break;
		case 0252 : /* ULSS */
			j = pop();
			i = pop();
			push ( i < j );
			break;
		case 0253 : /* ULEQ */
			j = pop();
			i = pop();
			push ( i <= j );
			break;
		case 0254 : /* UGTR */
			j = pop();
			i = pop();
			push ( i > j );
			break;
		case 0255 : /* UGEQ */
			j = pop();
			i = pop();
			push ( i >= j );
			break;
		case 0256 : /* TRA */
			transfer(next(),pop(),pop()); break;
		case 0257 : /* RDS */
			k = pop();
			i = next();
			do {
				stack[k++] = next2();
				--i;
				}
			while ( i >= 0 );
			break;
		case 0260 : /* LODFW */
			i = pop();
			restoreexpstack();
			push(i);
			break;
		case 0261 : /* LODFD */
			i = pop();
			j = pop();
			restoreexpstack();
			push(j);
			push(i);
			break;
		case 0262 : /* STORE */
			saveexpstack(); break;
		case 0263 : /* STOFV */
			i = pop();
			saveexpstack();
			stack[S++] = i;
			break;
		case 0264 : /* STOT */
			stack[S++] = pop(); break;
		case 0265 : /* COPT */
			i = pop();
			push(i);
			push(i);
			break;
		case 0266 : /* DECS */
			--S; break;
		case 0267 : /* PCOP */
			stack[L+next()] = S;
			sz = pop(); k = S+sz; adr = pop();
			while ( sz > 0 ) {
				stack[S++] = stack[adr++];
				--sz;
				}
			break;
		case 0270 : /* UADD */
			j = pop();
			i = pop();
			push(i+j);
			break;
		case 0271 : /* USUB */
			j = pop();
			i = pop();
			push(i-j);
			break;
		case 0272 : /* UMUL */
			j = pop();
			i = pop();
			push ( i * j );
			break;
		case 0273 : /* UDIV */
			j = pop();
			i = pop();
			push ( i / j );
			break;
		case 0274 : /* UMOD */
			j = pop();
			i = pop();
			push ( i % j );
			break;
		case 0275 : /* ROR */
			i = pop() % 16;
			j = pop();
			k = (j>>i) | ((j&(0xFFFF>>(16-i)))<<(16-i));
			push(k);
			break;
		case 0276 : /* SHL */
			i = pop() % 16;
			j = pop();
			k = j << i;
			push(k);
			break;
		case 0277 : /* SHR */
			i = pop() % 16;
			j = pop();
			k = j >> i;
			push(k);
			break;
		}
	    else if ( IR < 0340 ) switch ( IR ) {
		case 0300 : /* FOR1 */
			i = next();
			hi = pop();
			low = pop();
			adr = pop();
			k = PC + next2() - 2;
			if ( (!i && low <= hi) || (i && low >= hi) ) {
				stack[adr] = low;
				stack[S++] = adr;
				stack[S++] = hi;
				}
			else
				PC = k;
			break;
		case 0301 : /* FOR2 */
			hi = stack[S-1];
			adr = stack[S-2];
			ssz = snext();
#ifdef	TRACE
			trace(" FOR2: ssz = %d\n", ssz);
			trace("\t\t\t\tFOR2: hi = %d, stack[adr] = %d.\n",hi, stack[adr]);
#endif	TRACE
			k = PC + next2()-2;
			si = stack[adr]+ssz;
			if ( (ssz >= 0 && si>hi) || (ssz <= 0 && si < hi) )
				S -= 2;
			else {
				stack[adr] = si;
				PC = k;
				}
			break;
		case 0302 : /* ENTC */
			PC += next2()-2;
			k = pop();
			low = next2();
			hi = next2();
			stack[S++] = PC + 2*(hi-low) + 4;
			if ( k >= low && k <= hi )
				PC += 2*(k-low+1);
			PC += next2()-2;
			break;
		case 0303 : /* EXC */
			PC = stack[--S]; break;
		case 0304 : /* TRAP */
			trap(pop()); break;
		case 0305 : /* CHK */
			sk = pop();
			sj = pop();
			si = pop();
			push(si);
			if ( no_checks ) break;
			if ( si < sj || si > sk )
				crash("CHK fails: %o [%o..%o]",si,sj,sk);
			break;
		case 0306 : /* CHKZ */
			k = pop();
			i = pop();
			push(i);
			if ( no_checks ) break;
			if ( i > k )
				crash("CHKZ fails: %o > %o",i,k);
			break;
		case 0307 : /* CHKS */
			si = pop();
			push(si);
			if ( no_checks ) break;
			if ( si < 0 )
				crash("CHKS fails: -%o < 0",-si);
			break;
		case 0310 : /* EQL */
			push( pop() == pop() ); break;
		case 0311 : /* NEQ */
			push ( pop() != pop() ); break;
		case 0312 : /* LSS */
			sj = pop();
			si = pop();
			push( si < sj);
			break;
		case 0313 : /* LEQ */
			sj = pop();
			si = pop();
			push( si <= sj);
			break;
		case 0314 : /* GTR */
			sj = pop();
			si = pop();
			push( si > sj );
			break;
		case 0315 : /* GEQ */
			sj = pop();
			si = pop();
			push( si >= sj );
			break;
		case 0316 : /* ABS */
			si = pop();
			push( si >= 0 ? si : -si); break;
		case 0317 : /* NEG */
			si = pop();
			push(-si); break;
		case 0320 : /* OR */
			j = pop();
			i = pop();
			push( i | j );
			break;
		case 0321 : /* XOR */
			j = pop();
			i = pop();
			push ( i ^ j );
			break;
		case 0322 : /* AND */
			j = pop();
			i = pop();
			push( i & j );
			break;
		case 0323 : /* COM */
			push ( ~ pop() );
			break;
		case 0324 : /* IN */
			j = pop();
			i = pop();
			push( i <= 15 && ((0x8000>>i) & j) );
			break;
		case 0325 : /* LIN */
			push( 0xFFFF ); break;
		case 0326 : /* MSK */
			i = pop() % 16;
			push( 0xFFFF << (i-16) );
			break;
		case 0327 : /* NOT */
			push ( ! pop() );
			break;
		case 0330 : /* ADD */
			sj = pop();
			si = pop();
			push( si + sj );
			break;
		case 0331 : /* SUB */
			sj = pop();
			si = pop();
			push( si - sj );
			break;
		case 0332 : /* MUL */
			sj = pop();
			si = pop();
			push( si * sj );
			break;
		case 0333 : /* DIV */
			sj = pop();
			si = pop();
			push( si / sj );
			break;
		case 0334 : /* MOD */
			sj = pop();
			si = pop();
			push( si % sj );
			break;
		case 0335 : /* BIT */
			j = pop();
			k = 0x8000 >> j;
			push(k);
			break;
		case 0336 : /* NOP */
			break;
		case 0337 : /* MOVF */
			i = pop();
			j = pop() + pop()*4;
			k = pop() + pop()*4;
			while ( i > 0 ) {
				stack[k++] = stack[j++];
				--i;
				}
			break;
		}
	    else if ( IR < 0361 ) switch ( IR ) {
		case 0340 : /* MOV */
			k = pop();
			j = pop();
			i = pop();
			while ( k > 0 ) {
				stack[i++] = stack[j++];
				--k;
				}
			break;
		case 0341 : /* CMP */
			k = pop();
			j = pop();
			i = pop();
			if ( k == 0 ) {
				push(0);
				push(0);
				}
			else {
				while ( stack[i] != stack[j] &&
					k > 0 ) {
					++i; ++j; --k;
					}
				push(stack[i]);
				push(stack[j]);
				}
			break;
		case 0342 : /* DDT */
			crash("DDT not implemented"); break;
		case 0343 : /* REPL */
			crash("REPL not implemented"); break;
		case 0344 : /* BBLT */
			crash("BBLT not implemented"); break;
		case 0345 : /* DCH */
			crash("DCH not implemented"); break;
		case 0346 : /* UNPK */
			crash("UNPK not implemented"); break;
		case 0347 : /* PACK */
			crash("PACK not implemented"); break;
		case 0350 : /* GB */
			i = L;
			j = next();
			do {
				i = stack[i];
				--j;
				}
			while ( j > 0 );
			push(i);
			break;
		case 0351 : /* GB1 */
			push(stack[L]); break;
		case 0352 : /* ALLOC */
			i = pop();
			push(S);
			S += i;
			break;
		case 0353 : /* ENTR */
			i = next();
			S += i;
			break;
		case 0354 : /* RTN */
			S = L;
			L = stack[S+1];
			i = stack[S+2];
			if ( i & 0x8000 ) {
				G = stack[S];
				F = stack[G];
				PC = i & 0x7FFF;
				}
			else
				PC = i;
#ifdef TRACE
			if ( RANGE )
				trace("\n\nG = %o, L = %o, S = %o, F = %o\n",G,L,S,F);
#endif TRACE
			if ( !PC )	/* RTN von module 1 */
#ifdef TRACE
			{
				trace("\nend of execution\n");
				eox();
#endif TRACE
				exit(0);
#ifdef TRACE
			}
#endif TRACE
			break;
		case 0355 : /* CX */
			j = next();
			i = next();
#ifdef TRACE
			trace("\nG = %o, L = %o, S = %o, F = %o\n",G,L,S,F);
			act_mod = j;
			trace("\nmodule # %3o, proc # %3o\n",act_mod,i);
			showstack();
#endif
			mark(G,TRUE);
			G = stack[DFT+j];
			assert(G >= 0200);
			F = stack[G];
			assert( ! (G > 0200 && F == 0) );
			PC = 2*i;
			PC = next2();
			break;
		case 0356 : /* CI */
			i = next();
			mark(pop(),FALSE);
			PC = 2*i;
			PC = next2();
			break;
		case 0357 : /* CF */
			i = stack[S-1];
			mark(G,TRUE);
			j = i / 0x100;
			G = stack[DFT+j];
			F = stack[G];
			PC = 2 * ( i % 0x100 );
			PC = next2();
			break;
		case 0360 : /* CL */
			i = next();
#ifdef TRACE
			trace("module # %3o, proc # %3o\n",act_mod,i);
			showstack();
#endif
			mark(L,FALSE);
			PC = 2*i;
			PC = next2();
		}
	    else { /* CL1 - CL15 */
#ifdef TRACE
		trace("module # %3o, proc # %3o\n",act_mod,IR % 16);
		showstack();
#endif
		mark(L,FALSE);
		PC = 2*(IR % 16);
		PC = next2();
		}
#ifdef TRACE
	    if ( RANGE ) {
		    check();
		    trace("\n");
		    }
#endif
	    }
	while ( 1 );
}

crash ( s , p1 , p2 , p3 , p4 , p5 , p6 )
{
	CloseAll();
	fprintf(stderr,"\nLilith crashes ...\n");
	fprintf(stderr,s,p1,p2,p3,p4,p5,p6);
	fprintf(stderr,"\n");
	fprintf(stderr,"IR = %o\n",IR);
	fprintf(stderr,"F = %o\n",F);
	fprintf(stderr,"PC = %o\n",PC);
	backtrace(L,F);
	quit("CRASH");
}

backtrace (L,F)
register word	L,F;
{
	register word	S, i, PC;
	register word	looping;

	looping = 0;
	fprintf(stderr,"***    BACKTRACE    ***\n");
	fprintf(stderr,"S     L     F     PC\n");
	fprintf(stderr,"-----------------------\n");

	/*
	 *	Simulation aller moeglichen RTN-Befehle
	 */

	do {
		S = L;
		L = stack[S+1];
		i = stack[S+2];
		if ( i & 0x8000 ) {
			G = stack[S];
			F = stack[G];
			PC = i & 0x7FFF;
			}
		else
			PC = i;
		fprintf(stderr,"%5o %5o %5o %5o\n",S,L,F,PC);
		}
	while ( PC && looping++ < 100 );
}
