/* This is the solution to leetcode problem No.1: twoSum.
 * author: Jidong Xiao
 * */

#include <stdio.h>
#include <stdlib.h>
#include "harness.h"

/**
 *  * Note: The returned array must be malloced, assume caller calls free().
 *   */
int* twoSum(int* nums, int numsSize, int target, int* returnSize){
    int *ret = (int *)infiniti_malloc((sizeof(int))*2);
    int i, j;
    for(i=0; i<numsSize-1; i++) {
        for(j=i+1;j<numsSize;j++) {
            if ((nums[i] + nums[j]) == target) {
                ret[0] = i;
                ret[1] = j;
                goto out;
            }
        }
    }
out:
    /* this is ridiculous, the leetcode system expects us to set returnSize inside this function. */
    *returnSize = 2;
    return ret;
}

int main()
{
	int nums[4]={2,7,11,15};
	int i;
	int *ret;
	init_infiniti();
	int *returnSize = (int *)infiniti_malloc(sizeof(int));
	ret = twoSum(nums, 4, 9, returnSize);
	for(i=0; i<(*returnSize); i++) {
		printf("result ret[%d] is %d\n", i, ret[i]);
	}
	infiniti_free(ret);
	infiniti_free(returnSize);
	return 0;
}

/* vim: set ts=4: */

