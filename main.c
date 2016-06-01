#include <stdio.h>
#define SIZE 4

typedef struct kem {
    short count;
    float weight;
    float volume;
} quality;

int main(void)
{
//    int value1 = 44;
//    int arr[SIZE];
//    int value2 = 88;
//    int i;
//    printf("value1 = %d, value2 = %d\n", value1, value2);
//    for (i = -1; i <= SIZE; i++)
//        arr[i] = 2 * i + 1;
//    for (i = -1; i < 7; i++)
//        printf("%2d %d\n", i , arr[i]);
//    printf("value1 = %d, value2 = %d\n", value1, value2);
//    printf("address of arr[-1]: %p\n", &arr[-1]);
//    printf("address of arr[4]: %p\n", &arr[4]);
//    printf("address of value1: %p\n", &value1);
//    printf("address of value2: %p\n", &value2);


//    quality q = {.volume = 10};
//    int arr[5] = {[2] = 10};
//    printf("\narr[2] = %d", arr[2]);



    int arr[4][2] = {{2,4}, {1,3}, {6,8}, {5,7}};
    int *q[4];
    int a[4] = {1,2,3,4};
    int i, j =0;
    int (*p) [2];
    p = arr;

    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 2; j++)
        {
            printf("arr[%d][%d] = %p\t", i, j, &arr[i][j]);
        }
        printf("\n");
    }

    printf("\n arr = %p, \t arr + 1 = %p", arr[0], arr + 1);
    printf("\n arr[0] = %p, \t arr[0] + 1 = %p", arr[0], arr[0] + 1);
    printf("\n *arr = %p, \t *arr + 1 = %p", *arr, *arr + 1);
    printf("\n *arr[0] = %d", *arr[0]);
    printf("\n **arr = %d", **arr);
    printf("\n *(*(arr + 2) + 1) = %d", *(*(arr + 2) + 1));

    printf("\n\n\n p = %p, \t p + 1 = %p", p, p + 1);
    printf("\n p[0] = %p, \t p[0] + 1 = %p", p[0], p[0] + 1);
    printf("\n *p = %p, \t *p + 1 = %p", *p, *p + 1);
    printf("\n *p[0] = %d", *p[0]);
    printf("\n **p = %d", **p);
    printf("\n *(*(p + 2) + 1) = %d\n\n", *(*(p + 2) + 1));

    for(i = 0; i < 4; i++){
        q[i] = &a[i];
    }

    for(i = 0; i < 4; i++){
        printf("q[%d] = %d \t", i, *q[i]);
    }


    int x;
    if(x == 1)
        printf("hello");

    return 0;
}
