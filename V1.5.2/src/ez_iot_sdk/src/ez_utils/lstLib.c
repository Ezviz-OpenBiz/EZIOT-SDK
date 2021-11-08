/* lstLib.c - doubly linked list subroutine library */

/* Copyright 1984-2001 Wind River Systems, Inc. */

/*
DESCRIPTION
This subroutine library supports the creation and maintenance of a
doubly linked list.  The user supplies a list descriptor (type LIST)
that will contain pointers to the first and last nodes in the list,
and a count of the number of nodes in the list.  The nodes in the
list can be any user-defined structure, but they must reserve space
for two pointers as their first elements.  Both the forward and
backward chains are terminated with a NULL pointer.

The linked-list library simply manipulates the linked-list data structures;
no kernel functions are invoked.  In particular, linked lists by themselves
provide no task synchronization or mutual exclusion.  If multiple tasks will
access a single linked list, that list must be guarded with some
mutual-exclusion mechanism (e.g., a mutual-exclusion semaphore).

NON-EMPTY LIST:
.CS
   ---------             --------          --------
   | head--------------->| next----------->| next---------
   |       |             |      |          |      |      |
   |       |       ------- prev |<---------- prev |      |
   |       |       |     |      |          |      |      |
   | tail------    |     | ...  |    ----->| ...  |      |
   |       |  |    v                 |                   v
   |count=2|  |  -----               |                 -----
   ---------  |   ---                |                  ---
              |    -                 |                   -
              |                      |
              ------------------------
.CE

EMPTY LIST:
.CS
	-----------
        |  head------------------
        |         |             |
        |  tail----------       |
        |         |     |       v
        | count=0 |   -----   -----
        -----------    ---     ---
                        -	-
.CE

INCLUDE FILES: lstLib.h
*/

#include "lstLib.h"
#include <stdlib.h>
#include <stdio.h>
#include "ezconfig.h"
#include "mcuconfig.h"

#define HEAD	node.next		/* first node in list */
#define TAIL	node.previous		/* last node in list */

/*********************************************************************
*
* ezdev_lstLibInit - initializes lstLib module
*
* This routine pulls lstLib into the vxWorks image.
*
* RETURNS: N/A
*/
void ezdev_lstLibInit(void)
{
	return;
}

/*********************************************************************
*
* ezdev_lstInit - initialize a list descriptor
*
* This routine initializes a specified list to an empty list.
*
* RETURNS: N/A
*/

void ezdev_lstInit(LIST *pList)
{
	pList->HEAD  = NULL;
	pList->TAIL  = NULL;
	pList->count = 0;
}
/*************************************************************************
*
* ezdev_lstAdd - add a node to the end of a list
*
* This routine adds a specified node to the end of a specified list.
*
* RETURNS: N/A
*/

void ezdev_lstAdd(LIST *pList, NODE *pNode)
{
	ezdev_lstInsert(pList, pList->TAIL, pNode);
}
/**************************************************************************
*
* ezdev_lstConcat - concatenate two lists
*
* This routine concatenates the second list to the end of the first list.
* The second list is left empty.  Either list (or both) can be
* empty at the beginning of the operation.
*
* RETURNS: N/A
*/

void ezdev_lstConcat(LIST *pDstList,LIST *pAddList)
{
	if (pAddList->count == 0)		/* nothing to do if AddList is empty */
		return;

	if (pDstList->count == 0)
		*pDstList = *pAddList;
	else
	{
		/* both lists non-empty; update DstList pointers */

		pDstList->TAIL->next     = pAddList->HEAD;
		pAddList->HEAD->previous = pDstList->TAIL;
		pDstList->TAIL           = pAddList->TAIL;

		pDstList->count += pAddList->count;
	}

	/* make AddList empty */

	ezdev_lstInit (pAddList);
}
/**************************************************************************
*
* ezdev_lstCount - report the number of nodes in a list
*
* This routine returns the number of nodes in a specified list.
*
* RETURNS:
* The number of nodes in the list.
*/

int ezdev_lstCount(LIST *pList)
{
	return (pList->count);
}
/**************************************************************************
*
* ezdev_lstDelete - delete a specified node from a list
*
* This routine deletes a specified node from a specified list.
*
* RETURNS: N/A
*/

void ezdev_lstDelete(LIST *pList,NODE *pNode)
{
	/* modified by Hu Jiexun, Jan22,2009. */
	int firstNode = 0, lastNode = 0;

	if (pNode->previous == NULL)
	{
		if (pList->HEAD != pNode)
		{
			//printf("ezdev_lstDelete: HEAD = %p, pNode = %p, it's a dummy node!\n", pList->HEAD, pNode);
			return;
		}
		
		firstNode = 1;
	}
	else
	{
		if (pNode->previous->next != pNode)
		{
			//printf("ezdev_lstDelete: previous->next = %p, pNode = %p, it's a dummy node!\n", pNode->previous->next, pNode);
			return;
		}
		
		firstNode = 0;
	}

	if (pNode->next == NULL)
	{
		if (pList->TAIL != pNode)
		{
			//printf("ezdev_lstDelete: TAIL = %p, pNode = %p, it's a dummy node!\n", pList->TAIL, pNode);
			return;
		}
		
		lastNode = 1;
	}
	else
	{
		if (pNode->next->previous != pNode)
		{
			//printf("ezdev_lstDelete: next->previous = %p, pNode = %p, it's a dummy node!\n", pNode->next->previous, pNode);
			return;
		}

		lastNode = 0;
	}

	if (1 == firstNode)
	{
		pList->HEAD = pNode->next;
	}
	else
	{
		pNode->previous->next = pNode->next;
	}

	if (1 == lastNode)
	{
		pList->TAIL = pNode->previous;
	}
	else
	{
		pNode->next->previous = pNode->previous;
	}

	/* update node count */

	pList->count--;
}
/************************************************************************
*
* ezdev_lstExtract - extract a sublist from a list
*
* This routine extracts the sublist that starts with <pStartNode> and ends
* with <pEndNode> from a source list.  It places the extracted list in
* <pDstList>.
*
* RETURNS: N/A
*/

void ezdev_lstExtract(LIST *pSrcList,NODE *pStartNode, NODE *pEndNode,LIST *pDstList)
{
	int i;
	NODE *pNode;

	/* fix pointers in original list */

	if (pStartNode->previous == NULL)
		pSrcList->HEAD = pEndNode->next;
	else
		pStartNode->previous->next = pEndNode->next;

	if (pEndNode->next == NULL)
		pSrcList->TAIL = pStartNode->previous;
	else
		pEndNode->next->previous = pStartNode->previous;


	/* fix pointers in extracted list */

	pDstList->HEAD = pStartNode;
	pDstList->TAIL = pEndNode;

	pStartNode->previous = NULL;
	pEndNode->next       = NULL;


	/* count number of nodes in extracted list and update counts in lists */

	i = 0;

	for (pNode = pStartNode; pNode != NULL; pNode = pNode->next)
		i++;

	pSrcList->count -= i;
	pDstList->count = i;
}
/************************************************************************
*
* ezdev_lstFirst - find first node in list
*
* This routine finds the first node in a linked list.
*
* RETURNS
* A pointer to the first node in a list, or
* NULL if the list is empty.
*/

NODE *ezdev_lstFirst(LIST *pList)
{
	return (pList->HEAD);
}
/************************************************************************
*
* ezdev_lstGet - delete and return the first node from a list
*
* This routine gets the first node from a specified list, deletes the node
* from the list, and returns a pointer to the node gotten.
*
* RETURNS
* A pointer to the node gotten, or
* NULL if the list is empty.
*/

NODE *ezdev_lstGet(LIST *pList)
{
	NODE *pNode = pList->HEAD;

	if (pNode != NULL)                      /* is list empty? */
	{
		pList->HEAD = pNode->next;          /* make next node be 1st */

		if (pNode->next == NULL)            /* is there any next node? */
			pList->TAIL = NULL;             /*   no - list is empty */
		else
			pNode->next->previous = NULL;   /*   yes - make it 1st node */

		pList->count--;                     /* update node count */
	}

	return (pNode);
}
/************************************************************************
*
* ezdev_lstInsert - insert a node in a list after a specified node
*
* This routine inserts a specified node in a specified list.
* The new node is placed following the list node <pPrev>.
* If <pPrev> is NULL, the node is inserted at the head of the list.
*
* RETURNS: N/A
*/

void ezdev_lstInsert(LIST *pList,NODE *pPrev,NODE *pNode)
{
	NODE *pNext;

	if (pPrev == NULL)
	{				/* new node is to be first in list */
		pNext = pList->HEAD;
		pList->HEAD = pNode;
	}
	else
	{				/* make prev node point fwd to new */
		pNext = pPrev->next;
		pPrev->next = pNode;
	}

	if (pNext == NULL)
		pList->TAIL = pNode;		/* new node is to be last in list */
	else
		pNext->previous = pNode;	/* make next node point back to new */


	/* set pointers in new node, and update node count */

	pNode->next = pNext;
	pNode->previous	= pPrev;

	pList->count++;
}
/************************************************************************
*
* ezdev_lstLast - find the last node in a list
*
* This routine finds the last node in a list.
*
* RETURNS
* A pointer to the last node in the list, or
* NULL if the list is empty.
*/

NODE *ezdev_lstLast(LIST *pList)
{
	return (pList->TAIL);
}
/************************************************************************
*
* ezdev_lstNext - find the next node in a list
*
* This routine locates the node immediately following a specified node.
*
* RETURNS:
* A pointer to the next node in the list, or
* NULL if there is no next node.
*/

NODE *ezdev_lstNext(NODE *pNode)
{
	return (pNode->next);
}
/************************************************************************
*
* ezdev_lstNth - find the Nth node in a list
*
* This routine returns a pointer to the node specified by a number <nodenum>
* where the first node in the list is numbered 1.
* Note that the search is optimized by searching forward from the beginning
* if the node is closer to the head, and searching back from the end
* if it is closer to the tail.
*
* RETURNS:
* A pointer to the Nth node, or
* NULL if there is no Nth node.
*/

NODE *ezdev_lstNth(LIST *pList,int nodenum)
{
	NODE *pNode;

	/* verify node number is in list */

	if ((nodenum < 1) || (nodenum > pList->count))
		return (NULL);


	/* if nodenum is less than half way, look forward from beginning;
	   otherwise look back from end */

	if (nodenum < (pList->count >> 1))
	{
		pNode = pList->HEAD;

		while (--nodenum > 0)
			pNode = pNode->next;
	}

	else
	{
		nodenum -= pList->count;
		pNode = pList->TAIL;

		while (nodenum++ < 0)
			pNode = pNode->previous;
	}

	return (pNode);
}
/************************************************************************
*
* ezdev_lstPrevious - find the previous node in a list
*
* This routine locates the node immediately preceding the node pointed to 
* by <pNode>.
*
* RETURNS:
* A pointer to the previous node in the list, or
* NULL if there is no previous node.
*/

NODE *ezdev_lstPrevious(NODE *pNode)
{
	return (pNode->previous);
}
/************************************************************************
*
* ezdev_lstNStep - find a list node <nStep> steps away from a specified node
*
* This routine locates the node <nStep> steps away in either direction from 
* a specified node.  If <nStep> is positive, it steps toward the tail.  If
* <nStep> is negative, it steps toward the head.  If the number of steps is
* out of range, NULL is returned.
*
* RETURNS:
* A pointer to the node <nStep> steps away, or
* NULL if the node is out of range.
*/

NODE *ezdev_lstNStep(NODE *pNode, int nStep)
{
	int i;

	for (i = 0; i < abs (nStep); i++)
	{
		if (nStep < 0)
			pNode = pNode->previous;
		else if (nStep > 0)
			pNode = pNode->next;
		if (pNode == NULL)
			break;
	}
	return (pNode);
}

/************************************************************************
*
* ezdev_lstFind - find a node in a list
*
* This routine returns the node number of a specified node (the 
* first node is 1).
*
* RETURNS:
* The node number, or
* -1 if the node is not found.
*/

int ezdev_lstFind(LIST *pList,NODE *pNode)
{

	NODE *pNextNode;
	int index = 1;

	pNextNode = ezdev_lstFirst (pList);

	while ((pNextNode != NULL) && (pNextNode != pNode))
	{
		index++;
		pNextNode = ezdev_lstNext (pNextNode);
	}

	if (pNextNode == NULL)
		return (-1);
	else
		return (index);
}

/************************************************************************
*
* ezdev_lstFree - free up a list
*
* This routine turns any list into an empty list.
* It also frees up memory used for nodes.
*
* RETURNS: N/A
*
* SEE ALSO: free()
*/

void ezdev_lstFree(LIST *pList)
{
    if(NULL == pList)
    {
        printf("ezdev_lstFree NULL\n\n");
        return;
    }
	NODE *p1, *p2;

	if (pList->count > 0)
	{
		p1 = pList->HEAD;
		while (p1 != NULL)
		{
			p2 = p1->next;
			free ((char *)p1);
			p1 = p2;
		}
		pList->count = 0;
		pList->HEAD = pList->TAIL = NULL;
	}
}
