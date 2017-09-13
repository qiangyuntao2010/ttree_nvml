//  
// Ttree.h: header file  
//  
// Copyright (C) QYT..  All rights reserved  
//  
// This source is free to use as you like.  If you make  
// any changes please keep me in the loop.  Email them to  
// qiangyuntao2010.  
//  
// PURPOSE:  
//  
//  To implement thread as a C object  
//  
// REVISIONS  
// =======================================================  
// Date: 2017.8.1   
// Name: qiangyuntao  
// Description: File creation  
//////////////////////////////////////////////////////////////////////  
#ifndef _TTREE_H_  
#define _TTREE_H_  

#include <math.h>  
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <libpmem.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif

#define KV_SIZE 8
#define NODE_SIZE 2088
#define ITEM_NUM 128
#define MIN_KEY (128-2)
#define NODE_NUM (1024)
#define PATH "/home/qyt/ttree_nvml/my-file"
#define PMEM_LEN (1024*1024*1024*2UL)



namespace stx{

typedef struct _pmem_node
{
    bool is_empty;
    int  node_id;
    char *start_add;
}PMEM_NODE;


typedef struct _pm_meta_data
{
    PMEM_NODE *address[NODE_NUM];
}PM_META;

typedef struct _key_value_pair
{
	char key[KV_SIZE];
	char value[KV_SIZE];
}kv_t;

typedef struct meta_node
{
    unsigned long int  dm_node_num;
}DM_META;
  
typedef struct tagTTREENODE  
{   
    char *pmem_add;
    int pmem_id;
	tagTTREENODE *left;         // Left child pointer.  
    tagTTREENODE *right;        // Right child pointer.  
    int nItems;  // Internal node items.  
    kv_t item[ITEM_NUM];
    char bf;                   // Balabce factor(bf = right subtree height - left subtree height)     
} TTREENODE;  

template  <typename Key>
class ttree_default_set_traits
{
    static const bool selfverify = false;
    static const bool debug = false;
};

enum TraverseOrder  
{ 
    PreOrder,  
    InOrder,  
    PostOrder  
};

class CTtree{

private:

TTREENODE *root;

DM_META *dm_meta;

PM_META *pm_meta;

char* pmemaddr;

int is_pmem;

int times;

public:

CTtree()
{
    std::cout<<"Constructor is called"<<std::endl;

    root = NULL;

    if((dm_meta = (DM_META*)malloc(sizeof(DM_META))) == NULL)
    {
        perror("d_meta malloc error!");
    }

    if((pm_meta = (PM_META*)malloc(sizeof(PM_META))) == NULL)
    {
        perror("pm_meta malloc error!");
    }

    if((pmemaddr = (char*)malloc(sizeof(char))) == NULL)
    {
        perror("pmemaddr malloc error!");
    }

    times = 0;
}

public:
    
~CTtree()
{
    std::cout<<"Destructor is called!"<<std::endl;
}

public:

char *init_alloc()
{
    
    size_t mapped_len;
    int count;
    int fd;
    
    if ((pmemaddr = static_cast<char*>(pmem_map_file(PATH,PMEM_LEN,PMEM_FILE_CREATE,0666,&mapped_len,&is_pmem))) == NULL)
    {
        perror("pmem_map_file");
        exit(1);
    }
    fprintf(stdout,"THE PMEMADDR START ADDRESS IS : %p \n",pmemaddr);
    fprintf(stdout,"THE PMEM NODE SIZE IS %ld \n",sizeof(PMEM_NODE));
    fprintf(stdout,"THE DRAM NODE SIZE IS %ld \n",sizeof(TTREENODE));
    for(count = 0;count < NODE_NUM;count++)
    {
        pm_meta->address[count] = (PMEM_NODE*)malloc(sizeof(PMEM_NODE));
        if((pm_meta->address[count]->start_add = pmemaddr + count * sizeof(TTREENODE)) == NULL)
        {
            perror("pmem allocation error!");
            exit(1);
        }
        fprintf(stdout,"THE PMEM_NDOE ADDRESS IS %p \n",pm_meta->address[count]->start_add);
        pm_meta->address[count]->node_id = count;
        pm_meta->address[count]->is_empty = true;
        fprintf(stdout,"THE FUNCTION %s : NOW THE NUMBER PM_NODE NUM IS %d \n",__func__,count);
    }
    fprintf(stdout,"THE FUNCTON %s : THE NODE NUMBER IS %d\n",__func__,count);
    return pmemaddr;
}

char *find_empty_node(TTREENODE *pNode)
{
    int count = 0;
    for(;count < NODE_NUM;count++)
    {
        if(pm_meta->address[count]->is_empty == true)
        {
            fprintf(stdout,"THE FUNCTON %s : COUNT IS %d\n",__func__,count);
            pNode->pmem_id = count;
            pNode->pmem_add = pm_meta->address[count]->start_add;
            pm_meta->address[count]->is_empty = false;
            return pm_meta->address[count]->start_add;
        }
             
    }
    if(count == NODE_NUM - 1)
    {
        fprintf(stdout,"FUNCTON %s : ALL THE NODE IS FULL\n",__func__);
        return NULL;
    }
}

void pmem_memcpy(char* paddr,TTREENODE* srcaddr,int cc)
{
    if(is_pmem)
    {
        pmem_memcpy_persist(paddr,srcaddr,cc);
    }
    else
    {
        memcpy(paddr,srcaddr,cc);
        pmem_msync(paddr,cc);
    }
}

TTREENODE* FindMin(TTREENODE *pNode)  
{  
    if (pNode != NULL)  
    {  
        if (pNode->left == NULL)  
        { 
			fprintf(stdout,"Function %s:The node don not have left node\n",__func__);
            return pNode;  
        }  
        else  
        {  
            return FindMin(pNode->left);  
        }  
    }
    fprintf(stdout,"FUNCTION %s : THE NODE IS EMPTY",__func__);
    return NULL;  
}  

TTREENODE* FindMax(TTREENODE *pNode)  
{  
    if (pNode != NULL)  
    {  
        if (pNode->right == NULL)  
        {  
            return pNode; 
			fprintf(stdout,"Function %s:The node do not have right node\n",__func__);
        }  
        else  
        {  
            return FindMax(pNode->right);  
        }  
    }  
    fprintf(stdout,"FUNCTION %s : THE NODE IS EMPTY",__func__);
    return NULL;  
}  
  
/*int GetNodeSize() //P  
{  
    return m_nSize;  
} */ 
  
char *Find(const char *key)  
{  
    TTREENODE *pNode = root;  
    while (pNode != NULL)  
    {  
        int n = pNode->nItems;  
        char *keymin = pNode->item[0].key;  
        char *keymax = pNode->item[n > 0 ? n - 1 : 0].key;  
        int nDiff1 = keycompare(key, keymin);  
        int nDiff2 = keycompare(key, keymax);  
        if (nDiff1 >= 0 && nDiff2 <= 0)  
        {  
            int l = 0, r = n-1;  
            // Binary search.  
            while (l <= r)  
            {  
                int i = (l + r) >> 1;  
                char *itemkey = pNode->item[i].key;  
                int nDiff = keycompare(key, itemkey);  
                if (nDiff == 0)  
                {  
                    return pNode->item[i].value;  
                }  
                else if (nDiff > 0)  
                {   
                    l = i + 1;  
                }   
                else  
                {   
                    r = i - 1;                
                }  
            }  
            break;  
        }  
        else if (nDiff1 < 0)  
        {  
            pNode = pNode->left;  
        }  
        else if (nDiff2 > 0)  
        {  
            pNode = pNode->right;  
        }  
    }  
    fprintf(stdout,"The function %s : Can not find the k-v pair\n",__func__);
    return NULL;  
}  
  
int BalanceFactor(TTREENODE *pNode) const  
{  
    int l, r;     
    TTREENODE *p1, *p2;  
    l = r = 0;  
    p1 = p2 = pNode;  
    if (p1 != NULL)  
    {  
        while (p1->left != NULL)  
        {  
            p1 = p1->left;  
            l++;  
        }  
    }  
    if (p2 != NULL)  
    {  
        while (p2->right != NULL)  
        {  
            p2 = p2->right;  
            r++;  
        }  
    }  
    return (r - l);  
}  
  
int Depth()  
{  
    int l, r;     
    TTREENODE *p1, *p2;  
    l = r = 0;  
    p1 = p2 = root;  
    if (p1 != NULL)  
    {  
        while (p1->left != NULL)  
        {  
            p1 = p1->left;  
            l++;  
        }  
    }  
    if (p2 != NULL)  
    {  
        while (p2->right != NULL)  
        {  
            p2 = p2->right;  
            r++;  
        }  
    }  
    return Max(l, r);  
}  
  
const TTREENODE *GetMinNode()  //P
{  
    return FindMin(root);  
}  
  
const TTREENODE *GetMaxNode() //P  
{  
    return FindMax(root);  
}  
  
int Max( int a, int b ) const  
{  
    return (a > b ? a : b);  
}  
  
/** 
* Rotate T-tree node with left child.this is a single rotation for case LL. 
* Update balance factor, then return new root. 
*/  
TTREENODE *SingleRotateLeft(TTREENODE *pNode)  
{  
    TTREENODE *K = pNode->left;  
    pNode->left = K->right;  
    K->right = pNode;  
      
    // Adjust the balance factor.  
    pNode->bf = BalanceFactor(pNode);  
    K->bf = BalanceFactor(K);
    pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
    pmem_memcpy(K->pmem_add,K,sizeof(TTREENODE));
    return K;  // new root  
}     
  
/** 
* Rotate T-tree node with right child.this is a single rotation for case RR. 
* Update balance factor, then return new root. 
*/  
TTREENODE *SingleRotateRight(TTREENODE *pNode)  
{  
    TTREENODE *K = pNode->right;  
    pNode->right = K->left;  
    K->left = pNode;  
      
    // Adjust the balance factor.  
    pNode->bf = BalanceFactor(pNode);  
    K->bf = BalanceFactor(K);  
    pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
    pmem_memcpy(K->pmem_add,K,sizeof(TTREENODE));
    return K;  // new root  
}  
  
/** 
* Rotate T-tree node with left child.this is a double rotation for case LR. 
* Update balance factor, then return new root. 
*/   
TTREENODE *DoubleRotateLeft(TTREENODE *pNode)  
{  
    pNode->left = SingleRotateRight(pNode->left);  
      
    // Adjust the balance factor.  
    pNode->bf = BalanceFactor(pNode);  
      
    return SingleRotateLeft(pNode);  
}     
   
/** 
* Rotate T-tree node with right child.this is a double rotation for case RL. 
* Update balance factor, then return new root. 
*/   
TTREENODE *DoubleRotateRight(TTREENODE *pNode)  
{  
    pNode->right = SingleRotateLeft(pNode->right);  
      
    // Adjust the balance factor.  
    pNode->bf = BalanceFactor(pNode);  
      
    return SingleRotateRight(pNode);  
}     
  
void Insert(char *key, char *value)  
{  

    if (root == NULL)  
    {
        fprintf(stdout,"======%s=====\n",__func__);
        root = (TTREENODE*)malloc(sizeof(TTREENODE));
        root->pmem_add = init_alloc(); 
        strcpy(root->item[0].key,key);
        strcpy(root->item[0].value,value);
        root->nItems = 1;  
        root->left = NULL;  
        root->right = NULL;
    }  
    else 
    {  
        //int times = 0;
        fprintf(stdout,"%s : THE INSERTION TIMES IS %d\n",__func__,times++);
        TTREENODE *pNode = root; 
       // root->parent = NULL;
        pNode = root;  
        bool bRet = _insert(pNode, key, value);
        if (pNode != root)  
        {   
            root = pNode;  
        }  
    }  
}       
  
void FreeNode(TTREENODE *pNode)
{
    if(pNode != NULL)
    {
        dm_meta->dm_node_num--;
        pm_meta->address[pNode->pmem_id]->is_empty = true;
        free(pNode);  
        pNode = NULL;
    }
    else 
    {
        fprintf(stdout,"THE FUNCTION %s : THE NODE IS EMPTY\n",__func__);
    }
}  
  
TTREENODE *MallocNode()  
{  
    TTREENODE *pNode;
    pNode = static_cast<TTREENODE*>(malloc(sizeof(TTREENODE)));  
    memset(pNode, 0, sizeof(TTREENODE));  
    dm_meta->dm_node_num++;
    if((pNode->pmem_add = find_empty_node(pNode)) == NULL)
    {
       
        fprintf(stdout,"THE FUNCTION %s : NO MORE PMEM PLACE\n",__func__);
        exit(1);
    }
    return (pNode);  
}  
  
bool _insert(TTREENODE *pNode, char *key, char *value)  
{  
    int n = pNode->nItems;  
    char *keymin = pNode->item[0].key;  
    char *keymax = pNode->item[n > 0 ? n - 1 : 0].key;  
    int nDiff = keycompare(key, keymin);  
    if (nDiff <= 0)  
    {  
        TTREENODE *pLeftId = pNode->left;  
        if ((pLeftId == 0 || nDiff == 0 ) && pNode->nItems != ITEM_NUM)  
        {   
            for (int i = n; i > 0; i--)   
            {  
                strcpy(pNode->item[i].key,pNode->item[i-1].key);
                strcpy(pNode->item[i].value,pNode->item[i-1].value);
            }
            strcpy(pNode->item[0].key,key);
            strcpy(pNode->item[0].value,value);
            pNode->nItems += 1;
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return false;  
        }   
        if (pLeftId == 0)   
        {   
            pLeftId = MallocNode();  
            strcpy(pNode->item[0].key,key);
            strcpy(pNode->item[0].value,value);
            pLeftId->nItems += 1;          
            pNode->left = pLeftId; 
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            pmem_memcpy(pLeftId->pmem_add,pLeftId,sizeof(TTREENODE));
        }  
        else   
        {  
            TTREENODE *pChildId = pLeftId;  
            bool bGrow = _insert(pChildId, key, value);  
            if (pChildId != pLeftId)  
            {   
                //P 
                pNode->left = pLeftId = pChildId;  

            }  
            if (!bGrow)   
            {
                pmem_memcpy(pNode->left->pmem_add,pNode->left,sizeof(TTREENODE));
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                return false;  
            }  
        }  
        if (pNode->bf > 0)   
        {   
            pNode->bf = 0;
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return false;  
        }   
        else if (pNode->bf == 0)   
        {   
            pNode->bf = -1;     
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return true;  
        }   
        else   
        {   
            if (pLeftId->bf < 0)   
            {   
                pNode = SingleRotateLeft(pNode); // single LL turn  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            }   
            else   
            {   
                pNode = DoubleRotateLeft(pNode); // double LR turn    
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            }         
            return false;  
        }  
          
    }          
    nDiff = keycompare(key, keymax);  
    if (nDiff >= 0)  
    {  
        TTREENODE *pRightId = pNode->right;  
        if ((pRightId == 0 || nDiff == 0 ) && pNode->nItems != ITEM_NUM)  
        {   
            strcpy(pNode->item[n].key,key);
            strcpy(pNode->item[n].value,value);
            pNode->nItems += 1;  
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return false;  
        }   
        if (pRightId == 0)   
        {   
            pRightId = MallocNode();  
            strcpy(pRightId->item[0].key,key);
            strcpy(pRightId->item[0].value,value);
            pRightId->nItems += 1;  
            pNode->right = pRightId;
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            pmem_memcpy(pRightId->pmem_add,pRightId,sizeof(TTREENODE));

        }  
        else   
        {  
            TTREENODE *pChildId = pRightId;  
            bool bGrow = _insert(pChildId, key, value);  
            if (pChildId != pRightId)  
            {   //P
                pNode->right = pRightId = pChildId;  
            }  
            if (!bGrow)   
            {  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                pmem_memcpy(pRightId->pmem_add,pRightId,sizeof(TTREENODE));
                return false;  
            }  
        }  
        if (pNode->bf < 0)   
        {   
            pNode->bf = 0;  
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return false;  
        }   
        else if (pNode->bf == 0)   
        {   
            pNode->bf = 1;  
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return true;  
        }   
        else   
        {   
            if (pRightId->bf > 0)   
            {   
                pNode = SingleRotateRight(pNode); // single RR turn  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            }   
            else   
            {   
                pNode = DoubleRotateRight(pNode); // double RL turn   
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            }         
            return false;  
        }  
    }      
      
    int l = 1, r = n-1;  
    while (l < r)  
    {  
        int i = (l + r) >> 1;
        char* itemkey = pNode->item[i].key;
        nDiff = keycompare(key, itemkey);  
        if (nDiff > 0)  
        {   
            l = i + 1;  
        }   
        else  
        {   
            r = i;  
            if (nDiff == 0)  
            {   
                break;  
            }  
        }  
    }  
      
    // Insert before item[r]  
    if (n != ITEM_NUM)   
    {  
        for (int i = n; i > r; i--)   
        {  
            strcpy(pNode->item[i].key,pNode->item[i-1].key);
        }  
        strcpy(pNode->item[r].key,key);
        pNode->nItems += 1;  
        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
        return false;  
    }  
    else   
    {   
        char *reinsertId;  
        char *reinsertData;  
        // The right than the left subtree subtree weight, into the left equilibrium.  
        if (pNode->bf >= 0)   
        {   
            // Node in the value of the most left out,   
            // key inserted into its position in the r-1.  
            // Value will be inserted into the left of its left subtree.  
             strcpy(reinsertId,pNode->item[0].key);
             strcpy(reinsertData,pNode->item[0].value);
            for (int i = 1; i < r; i++)  
            {  
                
                strcpy(pNode->item[i-1].key,pNode->item[i].key);
                strcpy(pNode->item[i-1].value,pNode->item[i].value);
            }  
                strcpy(pNode->item[r-1].key,key);
                strcpy(pNode->item[r-1].value,value);
              
            return _insert(pNode, reinsertId, reinsertData);          
        }   
        else // The left than the right subtree subtree re-insert the right balance.  
        {   
            // Node in the value of the most right out,   
            // key inserted into the location of its r.  
            // The right value will be inserted to its right subtree.  
            strcpy(reinsertId,pNode->item[n-1].key);
            strcpy(reinsertData,pNode->item[n-1].value);
            for (int i = n-1; i > r; i--)   
            {
                strcpy(pNode->item[i].key,pNode->item[i-1].key);
                strcpy(pNode->item[i].value,pNode->item[i-1].value);
            } 
            strcpy(pNode->item[r].key,key);
            strcpy(pNode->item[r].value,value);
              
            return _insert(pNode, reinsertId, reinsertData);  
        }     
    }  
}    
  
void Clear()   
{  
    _earse(root);     
}  
  
void _earse(TTREENODE *pNode)   
{  
    if (pNode == NULL)  
    {  
        return;  
    }  
  
    _earse(pNode->left);  
      
    _earse(pNode->right);  
  
    FreeNode(pNode);  
}  
  
void Delete(char *key)  
{  
    TTREENODE *pNode = root;  
    int h = remove(pNode, key);  
    assert(h >= 0);  
    if (pNode != root)  
    {   
        root = pNode;  
    }  
}  
  
int BalanceLeftBranch(TTREENODE *pNode)  
{  
    if (pNode->bf < 0)  
    {   
        pNode->bf = 0;
        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
        return 1;  
    }   
    else if (pNode->bf == 0)  
    {   
       
        pNode->bf = 1;  
        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
        return 0;  
    }   
    else  
    {   
        TTREENODE *pRightId = pNode->right;  
        if (pRightId->bf >= 0)   
        {   
            pNode = SingleRotateRight(pNode); // single RR turn  
            if (pRightId->bf == 0)  
            {  
                pNode->bf = 1;  
                pRightId->bf = -1;
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                pmem_memcpy(pRightId->pmem_add,pRightId,sizeof(TTREENODE));
                return 0;  
            }  
            else  
            {  
                pNode->bf = 0;  
                pRightId->bf = 0;  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                pmem_memcpy(pRightId->pmem_add,pRightId,sizeof(TTREENODE));
                return 1;  
            }  
        }   
        else   
        {   
            pNode = DoubleRotateRight(pNode); // double RL turn   
            return 1;             
        }          
    }  
}  
  
int BalanceRightBranch(TTREENODE *pNode)  
{  
    if (pNode->bf > 0)  
    {   
        pNode->bf = 0;  
        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
        return 1;  
    }   
    else if (pNode->bf == 0)  
    {   
        pNode->bf = -1;  
        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
        return 0;  
    }   
    else  
    {   
        TTREENODE * pLeftId = pNode->left;  
        if (pLeftId->bf <= 0)   
        {   
            pNode = SingleRotateLeft(pNode); // single LL turn  
            if (pLeftId->bf == 0)  
            {  
                pNode->bf = -1;  
                pLeftId->bf = 1;  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                pmem_memcpy(pLeftId->pmem_add,pLeftId,sizeof(TTREENODE));
                return 0;  
            }  
            else  
            {  
                pNode->bf = 0;  
                pLeftId->bf = 0;  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                pmem_memcpy(pLeftId->pmem_add,pLeftId,sizeof(TTREENODE));
                return 1;  
            }  
        }   
        else   
        {   
            pNode = DoubleRotateLeft(pNode); // double LR turn    
            return 1;             
        }        
    }  
    return 0;  
}  
  
int remove(TTREENODE *pNode, char *key)  
{  
    int n = pNode->nItems;  
    char *keymin = pNode->item[0].key;  
    char *keymax = pNode->item[n > 0 ? n - 1 : 0].key;  
    int nDiff = keycompare(key, keymin);  
    if (nDiff <= 0)  
    {   
        TTREENODE *pLeftId = pNode->left;  
        if (pLeftId != 0)  
        {   
            TTREENODE *pChildId = pLeftId;  
            int h = remove(pChildId, key);  
            if (pChildId != pLeftId)  
            {   
                pNode->left = pChildId;  
            }  
            if (h > 0)  
            {   
                int ret = BalanceLeftBranch(pNode);
                return ret;
            }  
            else if (h == 0)  
            {   
                pmem_memcpy(pNode->pmem_add,pNode->left,sizeof(TTREENODE));
                return 0;  
            }  
        }  
        assert (nDiff == 0);  
    }  
    nDiff = keycompare(key, keymax);  
    if (nDiff <= 0)   
    {          
        for (int i = 0; i < n; i++)   
        {   
            if (strcmp(pNode->item[i].key,key) == 0)  
            {   
                if (n == 1)   
                {   
                    if (pNode->right == 0)   
                    {   
                        TTREENODE *pTempNode = pNode->left;  
                        FreeNode(pNode);  
                        pNode = pTempNode;
                        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                        return 1;  
                    }  
                    else if (pNode->left == 0)   
                    {   
                        TTREENODE *pTempNode = pNode->right;  
                        FreeNode(pNode);  
                        pNode = pTempNode;
                        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                        return 1;  
                    }   
                }   
                TTREENODE *pLeftId = pNode->left, *pRightId = pNode->right;  
                if (n <= MIN_KEY)  
                {   
                    if (pLeftId != 0 && pNode->bf <= 0)  
                    {    
                        while (pLeftId->right != 0)   
                        {   
                            pLeftId = pLeftId->right;  
                        }  
                        while (--i >= 0)   
                        {   
                            strcpy(pNode->item[i+1].key,pNode->item[i].key);
                            strcpy(pNode->item[i+1].value,pNode->item[i].value);
                        }
                        strcpy(pNode->item[0].key,pLeftId->item[pLeftId->nItems-1].key);
                        strcpy(pNode->item[0].value,pLeftId->item[pLeftId->nItems-1].value);
                        key = pNode->item[0].key;  
                        TTREENODE *pChildId = pLeftId;  
                        int h = remove(pChildId, pNode->item[0].key);  
                        if (pChildId != pLeftId)   
                        {   
                            pNode->left = pChildId;  
                        }  
                        if (h > 0)   
                        {  
                            h = BalanceLeftBranch(pNode);  
                        }  
                        pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                        return h;  
                    }   
                    else if (pNode->right != 0)   
                    {   
                        while (pRightId->left != 0)  
                        {   
                            pRightId = pRightId->left;  
                        }  
                        while (++i < n)  
                        {   
                            strcpy(pNode->item[i-1].key,pNode->item[i].key);
                            strcpy(pNode->item[i-1].value,pNode->item[i].value);
                        }
                        strcpy(pNode->item[n-1].key,pRightId->item[0].key);
                        strcpy(pNode->item[n-1].value,pRightId->item[0].value);
                        strcpy(key,pNode->item[n-1].key); 
                        TTREENODE *pChildId = pRightId;  
                        int h = remove(pChildId, key);  
                        if (pChildId != pRightId)  
                        {   
                            pNode->right = pChildId;  
                        }  
                        if (h > 0)  
                        {  
                            h = BalanceRightBranch(pNode);  
                        }  
                        return h;  
                    }  
                }  
                while (++i < n)  
                {   
                    strcpy(pNode->item[i-1].key,pNode->item[i].key);
                    strcpy(pNode->item[i-1].value,pNode->item[i].value);
                }  
                pNode->nItems -= 1;  
                pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
                return 0;  
            }  
        }  
    }  
    TTREENODE *pRightId = pNode->right;  
    if (pRightId != 0)  
    {   
        TTREENODE *pChildId = pRightId;  
        int h = remove(pChildId, key);  
        if (pChildId != pRightId)  
        {  
            pNode->right = pChildId;  
        }  
        if (h > 0)  
        {   
            return BalanceRightBranch(pNode);  
        }  
        else  
        {   
            pmem_memcpy(pNode->pmem_add,pNode,sizeof(TTREENODE));
            return h;  
        }  
    }  
    return -1;  
}  
  
bool IsEmpty( ) const  
{  
    return root == NULL;  
}  
  
int keycompare(const char* key1, char* key2)  
{
	int compare = strlen(key1) - strlen(key2);
	return compare == 0 ? strcmp(key1,key2) : compare;
	
}  
  
void TraverseTree(TraverseOrder order)  
{  
    switch (order)  
    {  
    case PreOrder:  
        PreOrderTraverse(root);  
        break;  
    case InOrder:  
        InOrderTraverse(root);  
        break;  
    case PostOrder:  
        PostOrderTraverse(root);  
        break;  
   /* case LevelOrder:  
        LevelOrderTraverse(root);  
        break;*/  
    }  
}  
  
void InOrderTraverse(TTREENODE *pNode) const  
{   
    if (pNode != NULL)  
    {   
        InOrderTraverse(pNode->left);   
        int nSize = pNode->nItems;  
        for (int i = 0; i < nSize; i++)  
        {  
            printf("%s ", pNode->item[i].key);  
        }   
        InOrderTraverse(pNode->right);   
    }  
}   
  
void PostOrderTraverse(TTREENODE *pNode) const  
{   
    if (pNode != NULL)  
    {   
        PostOrderTraverse(pNode->left);   
        PostOrderTraverse(pNode->right);   
        int nSize = pNode->nItems;  
        for (int i = 0; i < nSize; i++)  
        {  
            printf("%s ", pNode->item[i].key);  
        }  
    }  
}   
    
void PreOrderTraverse(TTREENODE *pNode) const  
{   
    if (pNode != NULL)  
    {   
        int nSize = pNode->nItems;  
        for (int i = 0; i < nSize; i++)  
        {  
            printf("%s ", pNode->item[i].key);  
        }  
        PreOrderTraverse(pNode->left);   
        PreOrderTraverse(pNode->right);   
    }    
}


};
}
  
#endif   
