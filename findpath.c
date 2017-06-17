#include <stdio.h>
#include <stdlib.h>
//#include <curses.h>
#include <math.h>

int arrived = 0;

typedef struct map
{
        int x;
        int y;
        char *elements;
} map_t;

void drawMap(map_t *map)
{
        int i = 0, j = 0;
        for (i = 0; i < map->x; i++)
        {
                for (j = 0; j < map->y; j++)
                {
                        printf("%c",map->elements[i*map->y+j]);
                }
                printf("\n");
        }
}

typedef struct coordinate
{
        int x;
        int y;
} coordinate_t;

int sameCoordinate(coordinate_t c1, coordinate_t c2)
{
        return c1.x == c2.x && c1.y == c2.y;
}

typedef struct candidate_coordinate
{
        coordinate_t co;
        int weight;
} candidate_coordinate_t;

typedef struct path
{
        int len;
        coordinate_t footprints[1024];
} path_t;

typedef struct tree_node
{
        coordinate_t co;
        struct tree_node *up;
        struct tree_node *down;
        struct tree_node *left;
        struct tree_node *right;
        struct tree_node *father;
} tree_node_t;

typedef struct queue
{
        tree_node_t *tree_node;
        struct queue *next;
} queue_t;

queue_t* newQueueNode(tree_node_t *tree_node)
{
        queue_t *Q = (queue_t *)malloc(sizeof(queue_t));
        if (!Q) return NULL;
        memset(Q, 0, sizeof(queue_t));
        Q->tree_node = tree_node;
        return Q;
}

void printQueue(queue_t *Q)
{
        printf("Current queue: ");
        while(Q)
        {
                printf("{%d,%d}",Q->tree_node->co.x,Q->tree_node->co.y);
                Q = Q->next;
        }
        printf("\n");
}

int inQueue(queue_t *Q, coordinate_t co)
{
        while(Q)
        {
                if (Q->tree_node->co.x == co.x && Q->tree_node->co.y == co.y)
                        return 1;
                Q = Q->next;
        }
        return 0;
}

void pushQueue(queue_t **Q, tree_node_t *tree_node)
{
        //printf("pushQueue() {%d,%d}\n",tree_node->co.x,tree_node->co.y);
        queue_t *node = newQueueNode(tree_node);
        if (node)
        {
                node->next = *Q;
                *Q = node;
        }
        //printQueue(*Q);
}

void popQueue(queue_t **Q, tree_node_t **tree_node)
{
        queue_t *q = *Q, *pq = *Q;
        if ((*Q)->next)
        {
                while(q->next)
                {
                        pq = q;
                        q = q->next;
                }
                pq->next = NULL;
                *tree_node = q->tree_node;
        }
        else
        {
                *tree_node = (*Q)->tree_node;
                *Q = NULL;
        }
        //printf("popQueue() {%d,%d}\n",(*tree_node)->co.x,(*tree_node)->co.y);
        //printQueue(*Q);
}

void insertUp(tree_node_t *father, tree_node_t *node)
{
        if (!father || !node)
        {
                printf("NULL father or node in insertUp()");
                return;
        }
        father->up = node;
        node->father = father;
        //node->up = node->down = node->left = node->right = NULL;
}
void insertDown(tree_node_t *father, tree_node_t *node)
{
        if (!father || !node)
        {
                printf("NULL father or node in insertDown()");
                return;
        }
        father->down = node;
        node->father = father;
}
void insertLeft(tree_node_t *father, tree_node_t *node)
{
        if (!father || !node)
        {
                printf("NULL father or node in insertLeft()");
                return;
        }
        father->left = node;
        node->father = father;
}
void insertRight(tree_node_t *father, tree_node_t *node)
{
        if (!father || !node)
        {
                printf("NULL father or node in insertRight()");
                return;
        }
        father->right = node;
        node->father = father;
}

int validCoordinate(map_t *map, coordinate_t co)
{
        return (co.x >=0 && co.x < map->x && co.y >=0 && co.y < map->y);
}

tree_node_t* newTreeNode(coordinate_t co)
{
        tree_node_t *T = NULL;
        T = (tree_node_t *)malloc(sizeof(tree_node_t));
        if (!T)
        {
                return NULL;
        }
        memset(T, 0, sizeof(tree_node_t));
        T->co.x = co.x;
        T->co.y = co.y;
        return T;
}

void DLRTree(tree_node_t *T)
{
        if(T)
        {
                printf("{%d,%d}",T->co.x,T->co.y);
                DLRTree(T->up);
                DLRTree(T->down);
                DLRTree(T->left);
                DLRTree(T->right);
        }
}

int hasNextBrother(tree_node_t *N)
{
        if (!N || !N->father) return 0;
        if (N->father->up == N) return N->father->down || N->father->left || N->father->right;
        else if (N->father->down == N) return N->father->left || N->father->right;
        if (N->father->left == N) return N->father->right;
        else return 0;
}

int hasNextUncle(tree_node_t *N)
{
        if (!N || !N->father) return 0;
        return hasNextBrother(N->father);
}

void printPrefixSpace(tree_node_t *N, int depth)
{
        char *buf = (char *)malloc(depth*3+1);
        if (!buf) return;
        memset(buf, ' ', depth*3);
        buf[depth*3] = 0;
        int i;
        buf[depth*3-1] = '_';
        buf[depth*3-2] = '_';
        buf[depth*3-3] = '|';
        for (i=depth; i>0; i--)
        {
                if (hasNextUncle(N)) buf[(i-2)*3] = '|';
                N = N->father;
        }
        printf("%s", buf);
}

void printPrefixSpaceLinux(tree_node_t *N, int depth)
{
	//"\342\224\202\302\240\302\240",
	//    "\342\224\234\342\224\200\342\224\200", 
	//    "\342\224\224\342\224\200\342\224\200",
        char *buf = (char *)malloc(depth*9+1);
        if (!buf) return;
        memset(buf, ' ', depth*9);
        buf[depth*9] = 0;
        char *tmp = (char *)malloc(depth*9+1);
        if (!tmp) return;
        memset(tmp, ' ', depth*9);
        tmp[depth*9] = 0;
        int i;

	if (hasNextBrother(N))
		sprintf(buf, "%s", "\342\224\234\342\224\200\342\224\200");
	else
		sprintf(buf, "%s", "\342\224\224\342\224\200\342\224\200");

        for (i=depth; i>0; i--)
        {
                if (hasNextUncle(N)) 
		{
			sprintf(tmp, "%s%s", "\342\224\202\302\240\302\240", buf);
		}
		else
		{
			sprintf(tmp, "%s%s", "   ", buf);
		}
		sprintf(buf, "%s", tmp);
                N = N->father;
        }
        printf("%s", buf);
}

void DLRTreeTree(tree_node_t *T, int depth)
{
        if(T)
        {
#ifdef __unix
#ifdef __linux
                printPrefixSpaceLinux(T, depth);
#endif
#else
                printPrefixSpace(T, depth);
#endif
                printf("(%d,%d)\n",T->co.x,T->co.y);
                DLRTreeTree(T->up, depth + 1);
                DLRTreeTree(T->down, depth + 1);
                DLRTreeTree(T->left, depth + 1);
                DLRTreeTree(T->right, depth + 1);
        }
}

void findPathInTree(tree_node_t *T, coordinate_t dst, path_t *path)
{
        if (arrived)
                return;
        path->footprints[path->len].x = T->co.x;
        path->footprints[path->len].y = T->co.y;
        path->len++;
        //printf("findPathInTree() add {%d,%d}, len %d, arrived %d\n",T->co.x,T->co.y,path->len,arrived);
        //drawPath(path);
        if (T->co.x == dst.x && T->co.y == dst.y)
        {
                arrived = 1;
                return;
        }
        if(0 && !T->up && !T->down && !T->left && !T->right)
        {
                path->len--;
                //printf("findPathInTree() del {%d,%d}, len %d, arrved %d\n",T->co.x,T->co.y,path->len,arrived);
                //drawPath(path);
                return;
        }
        if (T->up)
                findPathInTree(T->up, dst, path);
        if (T->down)
                findPathInTree(T->down, dst, path);
        if (T->left)
                findPathInTree(T->left, dst, path);
        if (T->right)
                findPathInTree(T->right, dst, path);

        if(!arrived)
        {
                path->len--;
                //printf("findPathInTree() del {%d,%d}, len %d, arrved %d\n",T->co.x,T->co.y,path->len,arrived);
                //drawPath(path);
                return;
        }
}

void markMap(map_t *map, coordinate_t co, char c)
{
        map->elements[co.x * map->y + co.y] = c;
        //printf("markMap() {%d,%d} to %c\n",co.x,co.y,c);
        //drawMap(map);
}

char getTreeNodeOri(tree_node_t *node)
{
        if (!node->father) return 'O';
        if (node->co.x > node->father->co.x) return 'v';
        else if (node->co.x < node->father->co.x) return '^';
        else if (node->co.y > node->father->co.y) return '>';
        else return '<';
}

void buildTree(map_t *map, tree_node_t *Tree)
{
        coordinate_t tmpCo;
        tree_node_t *T = Tree, *node;
        queue_t *Q = NULL;
        char ori;

        if (!map || !T)
        {
                printf("NULL map or T in buildTree()");
                return;
        }
        if (!canWalk(map, T->co))
        {
                //printf("buildTree() can't walk to {%d,%d}\n",T->co.x,T->co.y);
                return;
        }
        pushQueue(&Q, T);
        while(Q)
        {
                popQueue(&Q, &node);
                ori = getTreeNodeOri(node);
                markMap(map, node->co, ori);
                T = node;
                //add up child
                tmpCo.x = T->co.x - 1;
                tmpCo.y = T->co.y;
                if (canWalk(map, tmpCo) && !inQueue(Q, tmpCo))
                {
                        node = newTreeNode(tmpCo);
                        pushQueue(&Q, node);
                        insertUp(T, node);
                }
                //add down child
                tmpCo.x = T->co.x + 1;
                tmpCo.y = T->co.y;
                if (canWalk(map, tmpCo) && !inQueue(Q, tmpCo))
                {
                        node = newTreeNode(tmpCo);
                        pushQueue(&Q, node);
                        insertDown(T, node);
                }
                //add left child
                tmpCo.x = T->co.x;
                tmpCo.y = T->co.y - 1;
                if (canWalk(map, tmpCo) && !inQueue(Q, tmpCo))
                {
                        node = newTreeNode(tmpCo);
                        pushQueue(&Q, node);
                        insertLeft(T, node);
                }
                //add right child
                tmpCo.x = T->co.x;
                tmpCo.y = T->co.y + 1;
                if (canWalk(map, tmpCo) && !inQueue(Q, tmpCo))
                {
                        node = newTreeNode(tmpCo);
                        pushQueue(&Q, node);
                        insertRight(T, node);
                }
        }
}

map_t* initMap(int x, int y, coordinate_t blocks[], int num)
{
        map_t *map = NULL;
        int i = 0;
        map = (map_t *)malloc(sizeof(map_t));
        if (NULL == map)
                return NULL;
        memset(map,0,sizeof(map_t));
        map->x = x;
        map->y = y;
        map->elements = (char *)malloc(x*y*sizeof(char));
        if (NULL == map->elements)
        {
                free(map);
                return NULL;
        }
        memset(map->elements,1,x*y*sizeof(char));
        printf("initMap, num=%d\n",num);
        for (i = 0; i < num; i++)
        {
                if (!validCoordinate(map, blocks[i]))
                        continue;
                map->elements[blocks[i].x * map->y + blocks[i].y] = '1';
                //printf("initMap set block {%d,%d}\n",blocks[i].x,blocks[i].y);
        }
        return map;
}

void initPath(path_t *path)
{
        if (NULL == path)
        {
                printf("NULL path in initPath()!\n");
                return;
        }
        path->len = 0;
}

void drawPath(path_t *path)
{
        if (NULL == path)
        {
                printf("NULL path in drawPath()!\n");
                return;
        }
        int i = 0;
        for (i = 0; i < path->len; i++)
        {
                printf("{%d,%d}->",path->footprints[i].x,path->footprints[i].y);
        }
        printf("\n");
}

int inPath(path_t *path, coordinate_t co)
{
        int i;
        for (i = 0; i < path->len; i++)
        {
                if (sameCoordinate(path->footprints[i], co))
                        return 1;
        }
        return 0;
}

void drawPathOnMap(map_t *map, path_t *path)
{
        int i = 0, j = 0;
        coordinate_t co;
        char ch;
        for (i = 0; i < map->x; i++)
        {
                for (j = 0; j < map->y; j++)
                {
                        co.x = i;
                        co.y = j;
                        ch = map->elements[i*map->y+j];
                        if (inPath(path, co) || '1' == ch)
                                printf("%c",ch);
                        else
                                printf("%c",1);
                }
                printf("\n");
        }
}

void unmarkMap(map_t *map, coordinate_t co)
{
        map->elements[co.x * map->y + co.y] = 1;
        //printf("unmarkMap() {%d,%d}\n",co.x,co.y);
        //drawMap(map);
}

int canWalk(map_t *map, coordinate_t co)
{
        if (validCoordinate(map, co))
                return (map->elements[co.x * map->y + co.y] == 1);
        else
                return 0;
}

int compare(const void *co1, const void *co2)
{
        return ((candidate_coordinate_t *)co1)->weight - ((candidate_coordinate_t *)co2)->weight;
}

void sortCandidates(candidate_coordinate_t *co)
{
        qsort(co, 4, sizeof(candidate_coordinate_t), compare);
}

void printCandidates(candidate_coordinate_t *co)
{
        printf("candidates:{%d,%d,%d}{%d,%d,%d}{%d,%d,%d}{%d,%d,%d}\n",
               co[0].co.x,co[0].co.y,co[0].weight,
               co[1].co.x,co[1].co.y,co[1].weight,
               co[2].co.x,co[2].co.y,co[2].weight,
               co[3].co.x,co[3].co.y,co[3].weight);
}

void findPath(map_t *map, coordinate_t src, coordinate_t dst, path_t *path)
{
        candidate_coordinate_t co[4];
        memset(co,0xff,sizeof(candidate_coordinate_t)*4);
        int xd = 1, yd = 1;

        if (arrived)
                return;

        if (NULL == map)
        {
                printf("NULL map in findPath()!\n");
                return;
        }
        if (NULL == path)
        {
                printf("NULL path in findPath()!\n");
                return;
        }
        //printf("now at {%d,%d}\n",src.x,src.y);
        if (!validCoordinate(map, src) || !canWalk(map, src))
        {
                //printf("findPath() {%d,%d} invalid or can't walk\n",src.x,src.y);
                return;
        }

        markMap(map, src, '2');
        path->footprints[path->len].x = src.x;
        path->footprints[path->len].y = src.y;
        path->len++;

        if (src.x == dst.x && src.y == dst.y)
        {
                arrived = 1;
                return;
        }

        //if (dst.x < src.x)
        //        xd = -1;
        //if (dst.y < src.y)
        //        yd = -1;
        co[1].co.x = src.x + xd;
        co[1].co.y = src.y;
        co[1].weight = (dst.x-co[1].co.x)*(dst.x-co[1].co.x)+(dst.y-co[1].co.y)*(dst.y-co[1].co.y);
        co[2].co.x = src.x - xd;
        co[2].co.y = src.y;
        co[2].weight = (dst.x-co[2].co.x)*(dst.x-co[2].co.x)+(dst.y-co[2].co.y)*(dst.y-co[2].co.y);
        co[3].co.x = src.x;
        co[3].co.y = src.y + yd;
        co[3].weight = (dst.x-co[3].co.x)*(dst.x-co[3].co.x)+(dst.y-co[3].co.y)*(dst.y-co[3].co.y);
        co[4].co.x = src.x;
        co[4].co.y = src.y - yd;
        co[4].weight = (dst.x-co[4].co.x)*(dst.x-co[4].co.x)+(dst.y-co[4].co.y)*(dst.y-co[4].co.y);

        printCandidates(co);
        sortCandidates(co);
        printCandidates(co);

        //go direction 1
        if (validCoordinate(map, co[1].co) && canWalk(map, co[1].co))
        {
                findPath(map, co[1].co, dst, path);
                if (arrived)
                        return;
        }
        //go direction 2
        if (validCoordinate(map, co[2].co) && canWalk(map, co[2].co))
        {
                findPath(map, co[2].co, dst, path);
                if (arrived)
                        return;
        }
        //go direction 3
        if (validCoordinate(map, co[3].co) && canWalk(map, co[3].co))
        {
                findPath(map, co[3].co, dst, path);
                if (arrived)
                        return;
        }
        //go direction 4
        if (validCoordinate(map, co[4].co) && canWalk(map, co[4].co))
        {
                findPath(map, co[4].co, dst, path);
                if (arrived)
                        return;
        }
        if (!canWalk(map, co[1].co) &&
            !canWalk(map, co[2].co) &&
            !canWalk(map, co[3].co) &&
            !canWalk(map, co[4].co))
        {
                //printf("no way to go!\n");
                unmarkMap(map, src);
                path->len--;
        }
}

int randxy(int x, int y)
{
        return x+rand()%(y-x+1);
}

int inBlocks(coordinate_t *blocks, int num, coordinate_t co)
{
        int i;
        for(i=0;i<num;i++)
        {
                if(blocks[i].x == co.x && blocks[i].y == co.y)
                        return 1;
        }
        return 0;
}

void randonize(int* x,int* y,int* i,coordinate_t* blocks,coordinate_t* src,coordinate_t* dst)
{
        int j = 0;
        srand(time(0));
        *x = randxy(5,20);
        *y = randxy(5,20);
        *i = randxy(1,2*(int)(sqrt(*x**y)));
        for (j=0;j<*i;j++)
        {
                blocks[j].x = randxy(0,*x-1);
                blocks[j].y = randxy(0,*y-1);
                if (inBlocks(blocks, j, blocks[j]))
                {
                        j--;
                        continue;
                }
        }
        do
        {
                src->x = randxy(0,*x-1);
                src->y = randxy(0,*y-1);
        }while(inBlocks(blocks, *i, *src));
        do
        {
                dst->x = randxy(0,*x-1);
                dst->y = randxy(0,*y-1);
        }while(inBlocks(blocks, *i, *dst) || sameCoordinate(*src, *dst));
        printf("x=%d y=%d, i=%d\n",*x,*y,*i);
        printf("blocks ");
        for (j=0;j<*i;j++)
        {
                printf("{%d,%d}",blocks[j].x,blocks[j].y);
        }
        printf("\n");
        printf("src {%d,%d}\n",src->x,src->y);
        printf("dst {%d,%d}\n",dst->x,dst->y);
}

void main()
{
#ifdef __unix
#ifdef __linux
	printf("Linux\n");
#endif
#ifdef __sun
#ifdef __sparc
	printf("Sun SPARC\n");
#else
	printf("Sun X86\n");
#endif
#endif
#ifdef _AIX
	printf("AIX\n");
#endif
					      
#else
					      
//#ifdef __WINDOWS_
	printf("Windows\n");
//#endif
						  
#endif
#if 0
        printf("%s","\342\224\202\302\240\302\240");
        printf("%d\n",sizeof("\342\224\202\302\240\302\240"));
        printf("%d\n",strlen("\342\224\202\302\240\302\240"));
        printf("\342\224\234\342\224\200\342\224\200\n");
        printf("   \342\224\224\342\224\200\342\224\200\n");
        int c;
        for(c=0;c<255;c++)printf("%c",c);
        while(1);
        return;
#endif
        map_t *map = NULL;
        int i = 0, x = 0, y = 0;
        coordinate_t blocks[64] = {{2,0},{2,1},{2,2}};
        coordinate_t src = {3,0};
        coordinate_t dst = {0,4};
#if 0
        //input x y
        printf("x:");
        scanf("%d",&x);
        printf("y:");
        scanf("%d",&y);
        printf("x=%d y=%d\n",x,y);
        //input blocks
        while (1)
        {
                int x,y;
                printf("block x y:");
                scanf("%d %d",&x,&y);
                printf("{x=%d y=%d}\n",x,y);
                if (x < 0 || y < 0)
                        break;
                blocks[i].x = x;
                blocks[i].y = y;
                i++;
                //break;
        }
        //input src
        printf("src x y:");
        scanf("%d %d",&src.x,&src.y);
        printf("src x y:{%d,%d}\n",src.x,src.y);
        //input dst
        printf("dst x y:");
        scanf("%d %d",&dst.x,&dst.y);
        printf("dst x y:{%d,%d}\n",dst.x,dst.y);
#endif
START:
        randonize(&x,&y,&i,blocks,&src,&dst);
        path_t path;
        initPath(&path);
        //map = initMap(x, y, blocks, i);
        //drawMap(map);
        arrived = 0;
        //findPath(map, src, dst, &path);
        //drawPath(&path);
        //drawMap(map);

        //printf("Use tree to do it!\n");
        tree_node_t *T = NULL;
        map = initMap(x, y, blocks, i);
        drawMap(map);
        T = newTreeNode(src);
        buildTree(map, T);
        DLRTree(T);
        printf("\n");
        DLRTreeTree(T, 0);
        //printf("\n");
        findPathInTree(T, dst, &path);
        drawPath(&path);
        drawPathOnMap(map, &path);

        printf("Press any key to exit:");
        x = getch();
        if('c' == x) goto START;
        //printf("%c\n",x);
        //getchar();
}
