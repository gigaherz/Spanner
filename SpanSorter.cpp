#include "Spans.h"
#include "Profiler.h"

float Sign(float x)
{
	if(x==0)
		return 0;
	return x/abs(x);
}

void PtrSwap(Span **a,Span **b)
{
	Span *c=*b;
	*b=*a;
	*a=c;
}

//-1 = below
//0 = either unrelated or coincident
//1 = above
int SpanCompare(Span *a, Span *b, bool unrelated_returns_zero=false)
{
	if(a->degenerate())
		return 0;

	if(b->degenerate())
		return 0;

	//check if they overlap in the X axis
	if((a->start.X>=b->end.X)||(b->start.X>=a->end.X))
	{
		if(unrelated_returns_zero)
			return 0;
		//unrelated
		if((a->start.Z+a->end.Z)>(b->start.Z+b->end.Z))
			return 1;
		if((a->start.Z+a->end.Z)<(b->start.Z+b->end.Z))
			return -1;
		return 0;
	}

	//they overlap in X, we need aboveness testing
#define USE_REAL_ABOVENESS

#ifdef USE_REAL_ABOVENESS
#define max(x,y) (((x)>(y))?(y):(x))
#define min(x,y) (((x)<(y))?(y):(x))
	Span sa;
	Span sb;
	Span *ta=&sa;
	Span *tb=&sb;
	SpanClip(ta,a,max(a->start.X,b->start.X),min(a->end.X,b->end.X));
	SpanClip(tb,b,max(a->start.X,b->start.X),min(a->end.X,b->end.X));
#else
	Span *ta=a;
	Span *tb=b;
#endif

	//lame test for now
	float dzs=ta->start.Z-tb->start.Z;
	float dze=ta->end.Z-tb->end.Z;

	if((Sign(dzs)!=Sign(dze))&&(abs(dzs)<0.001)&&(abs(dze)<0.001))
	{
		printf("We have a problem!");
	}

	if((dzs+dze)>0)
		return 1;
	if((dzs+dze)<0)
		return -1;
	return 0;
}

Span *MergeSort(Span *first,int count)
{
	Span *second;

	if(count==1) //shouldn't happen unless the spanbuffer contains a single span
	{
		first->next=NULL;
		return first;
	}

	int n1=count/2;
	int n2=count-n1;

	second=first;
	int t=n1;
	while((t--)>0)
		second=second->next;

	if(n1>1)
		first=MergeSort(first,n1);
	else
		first->next=NULL;

	if(n2>1)
		second=MergeSort(second,n2);
	else
		second->next=NULL;

	//merge
	Span *result_start=NULL;
	Span *result_end=NULL;
	
	//first step done manually
	switch(SpanCompare(first,second))
	{
	case -1:
		result_start=first;
		result_end=first;
		first=first->next;
		break;
	case 0:
		result_start=first;
		result_end=second;

		first=first->next;
		second=second->next;

		result_start->next=result_end;
		break;
	case 1:
		result_start=second;
		result_end=second;
		second=second->next;
		break;
	}

	while((first!=NULL)&&(second!=NULL))
	{
		switch(SpanCompare(first,second))
		{
		case -1:
			result_end->next=first;
			result_end=first;
			first=first->next;
			break;
		case 0:
			result_end->next=first;
			result_end=first;
			first=first->next;

			result_end->next=second;
			result_end=second;
			second=second->next;
			break;
		case 1:
			result_end->next=second;
			result_end=second;
			second=second->next;
			break;
		}
	}

	while((first!=NULL))
	{
		result_end->next=first;
		result_end=first;
		first=first->next;
	}

	while((second!=NULL))
	{
		result_end->next=second;
		result_end=second;
		second=second->next;
	}

	result_end->next=NULL;

	return result_start;
}

/******************************************************************************************\
********************************************************************************************
\******************************************************************************************/

// "x depends on y" means x is ABOVE y, 
#define depends(x,y) (SpanCompare(x,y)>0)

void AddToDependencyGraph(Span *root,Span *node)
{
	//check if node is above root child
	if(root->tree_child==NULL);

	//if(depends(node,root->tree_child))
}

void AddToSortedList(Span **first, Span *root)
{
	if(root->tree_child!=NULL)
		AddToSortedList(first,root->tree_child);

	if(root->tree_brother!=NULL)
		AddToSortedList(first,root->tree_brother);

	root->next=(*first);
	*first=root;
}


Span *DependencyTreeSort(Span *first,int count)
{
	Span root;
	root.tree_brother=NULL;
	root.tree_child=NULL;

	//add all the nodes to the list
	while(first!=NULL)
	{
		AddToDependencyGraph(&root,first);
		first=first->next;
	}

	Span *firstInList=NULL;

	if(root.tree_child!=NULL)
		AddToSortedList(&firstInList,root.tree_child);

	if(root.tree_brother!=NULL)
		AddToSortedList(&firstInList,root.tree_brother);

	return firstInList;
}

void SortSpanBuffer(SpanBuffer *sb)
{
	if(sb->first==NULL)
		return;

	//link the spans together
	int num=1;
	Span *l=sb->first->span;
	for(SpanHolder *it=sb->first->next;it!=NULL;it=it->next)
	{
		l->next=it->span;
		l=it->span;
		num++;
	}
	l->next=NULL;
	//send the spans to the mergesort function
	sb->firstspan=MergeSort(sb->first->span,num);
}

void SortScanline(Scanline *sl)
{
	for(int i=0;i<sl->nbuffers;i++)
	{
		SortSpanBuffer(sl->buffers+i);
	}

}

void SortScreenBuffer(ScreenBuffer *sb)
{
	Profiler_FunctionEnter(SortScreenBuffer);
	for(int y=0;y<sb->height;y++)
	{
		SortScanline(sb->lines+y);
	}
	Profiler_FunctionExit();
}

