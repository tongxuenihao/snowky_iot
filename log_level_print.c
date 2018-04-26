/*************************************************************************
	> File Name: log_level_print.c
	> Author: 
	> Mail: 
	> Created Time: Tue 24 Apr 2018 11:46:10 AM CST
 ************************************************************************/
#include "FreeRTOS.h"
#include "task.h"
#include "va_list.h"
#include "log_level_print.h"

#define A_PRINTF printf
static int _log_level = DEFAULT_LOG_PRINTF_LEVEL;

#undef SPACE
#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */

#if 0
void print_hex(unsigned char *a, int b)  
{
	printf("%s\n", __func__);
    int count_tmp; 
    for(count_tmp = 0;count_tmp < b;)
    {
        printf("%02x,",a[count_tmp++]); 
        if(count_tmp % 16 == 0)
        { 
            printf("\n"); 
        } 
    }
    printf("\n"); \
}
#endif


static int __do_div(long n, int base) 
{ 
int __res;
__res = ((unsigned long) n) % (unsigned) base; 
n = ((unsigned long) n) / (unsigned) base; 
return __res; 
}

static inline int isdigit(int ch)
{
	return (ch >= '0') && (ch <= '9');
}

static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

static char *number(char *str, long num, int base, int size, int precision,
		    int type)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

	char tmp[66];
	char c, sign, locase;
	int i;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return NULL;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = (digits[__do_div(num, base)] | locase);
			//tmp[i++] = (digits[__do_div(num, base)] | locase);
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base == 8)
			*str++ = '0';
		else if (base == 16) {
			*str++ = '0';
			*str++ = ('X' | locase);
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}

static int log_sprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char *str;
	const char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str = buf; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
	      repeat:
		++fmt;		/* this also skips first '%' */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char)va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
                        if(s == NULL){
                                continue;
                        }
			//len = strnlen(s, precision);
			len = strlen(s);

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str,
				     (unsigned long)va_arg(args, void *), 16,
				     field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int *ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN)
				num = (short)num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';

	return str - buf;
}

void set_log_printf_level(int log_level)
{
        if((log_level > -1) && (log_level < MAX_LOG_LEVEL)){
                _log_level = log_level + '0';
        }else{
                _log_level = DEFAULT_LOG_PRINTF_LEVEL;
        }
}

/**
* @brief        log_printf 
*
* @param        format
* @param        ...
*/
void log_printf(char *format,...)
{
        va_list ap;
        //int pos;
        char str[MAX_PRINTF_BUF] = "0";

        va_start(ap,format);

        if(format == NULL){
                return ;
        }

        if(strlen(format) > 3){
                if((format[0] == '<') && (format[2] == '>')){
                        if((format[1] > ('0' - 1)) && (format[1] < (MAX_LOG_PRINTF_LEVEL + 1))){
                                if(!(format[1] > _log_level)){
                                        log_sprintf(str,format,ap);
                                        //log_sprintf(format,ap);
                                        A_PRINTF("%s",str);

                                        return ;
                                }
                        }
                }
        }

        if(_log_level > (DEFAULT_LOG_PRINTF_LEVEL - 1)){
                log_sprintf(str,format,ap);
                //log_sprintf(format,ap);
                A_PRINTF("<%d>%s",DEFAULT_LOG_LEVEL,str);
        }
}
#if 0
int main()
{
        set_log_printf_level(2);

        log_printf(LOG_DEBUG"%d-->%d\n",3,3); 
        log_printf(LOG_INFO"%d-->%d\n",2,2); 
        log_printf(LOG_WARNING"%d-->%d\n",1,1); 
        log_printf(LOG_ALERT"%d-->%d\n",0,0); 
        log_printf("<%d-->%d\n",0,0); 
        
}
#endif

