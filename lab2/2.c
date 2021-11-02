#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


typedef struct {
    uint32_t num[35];
    int if_overflow;
    int lengthNum;
} uint1024_t;


uint1024_t from_uint(unsigned int x) {
    uint1024_t tmp;
    for(size_t i = 0; i < 35; ++i)
        tmp.num[i] = 0;
    tmp.num[0] = x % 1000000000;
    tmp.lengthNum = tmp.num[0] != 0;
    tmp.num[1] =  (uint32_t)(x / 1000000000);
    tmp.lengthNum += tmp.num[1] != 0;
    return tmp;
}


uint1024_t add_op(uint1024_t x, uint1024_t y) {
    int flag = 0;
    for (size_t i = 0; i < 35; ++i) {
        x.num[i] += flag + y.num[i];
        flag = x.num[i] > 999999999;
        if (flag) {
            if (i == 34) {
                x.if_overflow = 1;
                break;
            }
            x.num[i] -= 1000000000;
        }
    }
    for (size_t j = 34; j > -1; --j) {
        if (x.num[j] != 0) {
            x.lengthNum = j + 1;
            break;
        }
    }
    return x;
}


uint1024_t subtr_op(uint1024_t x, uint1024_t y) {
    int flag = 0;
    for (int i = 0; i < 35; ++i) {
        x.num[i] -= flag + y.num[i];
        flag = (int)x.num[i] < 0;
        if (flag)
            x.num[i] += 1000000000;
    }
    for (size_t j = 34; j > -1; --j) {
        if (x.num[j] != 0) {
            x.lengthNum = j + 1;
            break;
        }
    }
    return x;
}


uint1024_t mult_op(uint1024_t x, uint1024_t y) {
    uint1024_t z;
    for(size_t i = 0; i < 35; ++i)
        z.num[i] = 0;

    for (size_t i = 0; i < 35; ++i)
        for (size_t j = 0, flag = 0; 35 > i + j; ++j) {
            long long cur = z.num[i + j] + x.num[i] * 1ll * y.num[j] + flag;
            z.num[i + j] = (int)(cur % 1000000000);
            flag = (int)(cur / 1000000000);
            if (flag != 0 && i == 34) {
                break;
            }
        }
    for (size_t k = 34; k >= 0; --k) {
        if (z.num[k] != 0) {
            z.lengthNum = k + 1;
            break;
        }
    }
    return z;
}


void printf_value(uint1024_t x) {
    if (x.if_overflow != 1 && (int)x.num[x.lengthNum - 1] > 0) {
        printf("%u", x.num[x.lengthNum - 1]);
    } else if (x.if_overflow != 1 && x.num[x.lengthNum - 1] == 0) {
        printf("0\n");
        return;
    } else {
        printf("Undefined Behavior\n");
        return;
    }
    for(int i = x.lengthNum - 2; i > -1; --i) {
        printf("%09u", x.num[i]);
    }
    printf("\n");
}


void scanf_value(uint1024_t *x) {
    for(size_t i = 0; i < 35; ++i)
        x->num[i] = 0;
    x->lengthNum = 0;
    char* currNum = (char*)(calloc(315, sizeof(char)));
    scanf("%s", currNum);

    for(size_t j = 0; j < (size_t)strlen(currNum); ++j) {
        if (!isdigit(currNum[j])) {
            printf("Please write a number.");
            exit(1);
        }
    }

    for(int k = (int)strlen(currNum); k > 0; k -= 9) {
        currNum[k] = 0;
        x->num[x->lengthNum] = (uint32_t)(atoi(k >= 9 ? currNum + k - 9 : currNum));
        x->lengthNum += 1;
    }
    x->if_overflow = 0;
}


int main() {
    uint1024_t num1, num2;
    unsigned int x2 = 4003043439;
    printf("Number with type uint1024_t from unsigned int: ");
    printf_value(from_uint(x2));

    scanf_value(&num1);
    scanf_value(&num2);

    uint1024_t num_sum = add_op(num1, num2);
    uint1024_t num_sub = subtr_op(num1, num2);
    uint1024_t num_mult = mult_op(num1, num2);

    printf("Result of addition: ");
    printf_value(num_sum);
    printf("Result of subtraction: ");
    printf_value(num_sub);
    printf("Result of multiplication: ");
    printf_value(num_mult);
    return 0;
}
