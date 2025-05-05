#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main() {
    int x;
    float y;
    char c;
    bool i;
    i = ((!(((11 >= 10) && (2 < 3)) || (5 == 5))) || ((!(5 == 5)) || (2 < 3)));
    c = 'A';
    printf("%d\n", c);
    printf("i is: \n");
    printf("%d\n", i);
    if (i) {
        printf("i is true\n");
    } else {
        printf("i is false\n");
    }
    x = 10;
    y = (x + 120.500000);
    printf("y is: \n");
    printf("%d\n", y);
    printf("Enter value for x: ");
    /* Warning: Unknown variable x */
    printf("x is: \n");
    printf("%d\n", x);
    printf("Hello, world!\n");
    if ((x < 10)) {
        y = 5;
    }
    if ((y > 100)) {
        printf("y is greater than 10 and less than 100000\n");
        if ((y > 100)) {
            printf("y is greater than 100\n");
        } else {
            printf("y is less than or equal to 100\n");
        }
        printf("here is the end of the exp\n");
    } else {
        printf("y is less than or equal to 10\n");
    }
    x = ((5 + 3) * 2);
    y = (x / 4);

    return 0;
}
