#ifndef LINKLIST_H
#define LINKLIST_H

#include <stdlib.h>

template <class TYPE>
class LinkListNode {
public:
	TYPE data ;
	class LinkListNode<TYPE> *next ;
	class LinkListNode<TYPE> *prev ;

public:
	LinkListNode() {
		next = prev = NULL ;
	};
	inline TYPE NextData(void) {
		return next->data ;
	};
	inline TYPE Data(void) {
		return data ;
	};
	inline TYPE PrevData(void) {
		return prev->data ;
	};
	inline class LinkListNode<TYPE> *Next(void) {
		return next ;
	};
	inline class LinkListNode<TYPE> *Prev(void) {
		return prev ;
	};
} ;

template <class TYPE>
class ArrayList ;

template <class TYPE>
class LinkList {
public:
	LinkListNode<TYPE> *head ;
	LinkListNode<TYPE> *curr ;
	int size ;
private:
	LinkListNode<TYPE> *bookmark ;
public:
	LinkList() {
		curr = head = NULL ;
		bookmark = NULL ;
		size = 0 ;
	};

	LinkList(ArrayList<TYPE>& alist, int asize) {
		curr = head = NULL ;
		bookmark = NULL ;
		size = 0 ;
		Init(alist, asize) ;
	};

	void Init(ArrayList<TYPE>& alist, int asize) {
		int i ;

		while(size) DeleteNodeFromHead() ;
		curr = head = NULL ;
		bookmark = NULL ;
		size = 0 ;
		for(i = 0; i < asize; i++) {
			Append(alist[i]) ;
		}
	};

	inline void CleanUp(void) {
		while(size) DeleteNodeFromHead() ;
		curr = head = NULL ;
		bookmark = NULL ;
		size = 0 ;
	};

	~LinkList() {
		CleanUp() ;
	};

	inline void Clone(LinkList<TYPE> &llist) {
		CleanUp() ;
		Append(llist) ;
	};

	inline void Append(LinkList<TYPE> &llist) {
		TYPE data ;

		if(llist.Size()) {
			data = llist.SeekHead() ;
			do {
				Append(data) ;
				data = llist.Next() ;
			} while(!llist.Sol()) ;
		}
	};

	inline void Append(LinkList<TYPE> *llist) {
		TYPE data ;

		if(llist->Size()) {
			data = llist->SeekHead() ;
			do {
				Append(data) ;
				data = llist->Next() ;
			} while(!llist->Sol()) ;
		}
	};

	int Size() {
		return size ;
	};

	inline LinkListNode<TYPE> *NewNodeFromTail(void) {

		if(!head) {
			head = new LinkListNode<TYPE> ;
			head->next = head ;
			head->prev = head ;
			curr = head ;
			size++ ;

			return head ;
		} else {
			LinkListNode<TYPE> *newNode ;

			newNode = new LinkListNode<TYPE> ;
			newNode->next = head ;
			newNode->prev = head->prev ;
			head->prev = newNode ;
			newNode->prev->next = newNode ;
			size++ ;

			return newNode ;
		}
	};

	inline LinkListNode<TYPE> *NewNodeFromHead(void) {

		if(!head) {
			head = new LinkListNode<TYPE> ;
			head->next = head ;
			head->prev = head ;
			curr = head ;
			size++ ;

			return head ;
		} else {
			LinkListNode<TYPE> *newNode ;

			newNode = new LinkListNode<TYPE> ;
			newNode->next = head ;
			newNode->prev = head->prev ;
			head->prev = newNode ;
			newNode->prev->next = newNode ;
			size++ ;
			head = newNode ;

			return newNode ;
		}
	};

	inline LinkListNode<TYPE> *NewNodeFromCurr(void) {

		LinkListNode<TYPE> *newEntry ;
	
		if(!curr) return NULL ;

		newEntry = new LinkListNode<TYPE> ;
		newEntry->next = curr->next ;
		newEntry->next->prev = newEntry ;
		newEntry->prev = curr ;
		curr->next = newEntry ;
		curr = newEntry ;
		size++ ;

		return newEntry ;
	} ;

	inline LinkListNode<TYPE> *DeleteNodeFromTail(void) {

		LinkListNode<TYPE> *tail ;

		if(!head) {
			return NULL ;
		} else {

			tail = head->prev ;
			tail->prev->next = head ;
			tail->next->prev = tail->prev ;
			size-- ;
			delete tail ;

			if(!size) curr = head = NULL ;

			if(head)
				return head->prev ;
			else return NULL ;
		}
	};

	inline LinkListNode<TYPE> *DeleteNodeFromHead(void) {

		LinkListNode<TYPE> *nextHead ;
		if(!head) {
			return NULL ;
		} else {

			nextHead = head->next ;
			head->prev->next = head->next ;
			head->next->prev = head->prev ;
			size-- ;
			delete head ;

			if(size)
				head = nextHead ;
			else curr = head = NULL ;

			return head ;
		}
	};

	inline LinkListNode<TYPE> *DeleteNodeFromCurr(void) {

		LinkListNode<TYPE> *nextCurr ;

		if(!head) {
			return NULL ;
		} else {
			nextCurr = curr->next ;
			curr->prev->next = curr->next ;
			curr->next->prev = curr->prev ;
			size-- ;

			if(curr == head) head = curr->next ;

			delete curr ;

			curr = nextCurr ;

			if(!size)
				curr = head = NULL ;

			return curr ;
		}
	} ;

	inline void Append(TYPE data) {
		NewNodeFromTail()->data = data ;
	};

	inline void Push(TYPE data) {
		NewNodeFromHead()->data = data ;
	};

	inline TYPE Pop(void) {
		TYPE rdata ;

		if(head) {
			rdata = head->data ;
			DeleteNodeFromHead() ;
		}

		return rdata ;
	}

	inline TYPE Data(void) {
		return curr->Data() ;
	};

	inline TYPE Next(void) {
		curr = curr->next ;
		return curr->data ;
	};

	inline TYPE Prev(void) {
		curr = curr->prev ;
		return curr->data ;
	};

	inline TYPE SeekHead(void) {
		curr = head ;
		return head->data ;
	};

	inline TYPE SeekTail(void) {
		curr = head->prev ;
		return curr->data ;
	};

	inline TYPE SeekPos(int i) {
		curr = head ;
		for(i = 0; i < i; i++) {
			curr = curr->next ;
		}
		return curr->data ;
	};

	inline TYPE SeekData(TYPE sData) {
		curr = head ;
		do {
			if(curr->data == sData) {
				return curr->data ;
			}
			curr = curr->next ;
		} while(curr != head) ;
		return NULL ;
	} ;

	inline void Insert(TYPE data) {
		NewNodeFromCurr()->data = data ;
	} ;

	inline void Delete(void) {
		DeleteNodeFromCurr() ;
	} ;

	inline void Delete(TYPE data) {
		SeekData(data) ;
		Delete() ;
	} ;

	inline int Eol(void) {
		if(curr->next == head) return 1 ;
		return 0 ;
	} ;

	inline int Sol(void) {
		if(curr == head) return 1 ;
		return 0 ;
	} ;

	inline void Sort(int n, int (*compare)(TYPE *, TYPE *)) {
		ArrayList<TYPE> arrayList(*this) ;
		int i ;

		arrayList.Sort(n, compare) ;
		while(size) DeleteNodeFromHead() ;
		for(i = 0; i < arrayList.size; i++) {
			Append(arrayList[i]) ;
		}
	};

	inline void BookMark(void) {
		bookmark = curr ;
	};

	inline void Revert(void) {
		if(bookmark) curr = bookmark ;
		else curr = head ;
	};

	inline LinkList<TYPE> &operator=(LinkList<TYPE> &llist) {

		Clone(llist) ;
		return *this ;
	};
} ;

template <class TYPE>
class ArrayList {
public:
	TYPE *list ;
	int size ;
public:
	ArrayList() {
		list = new TYPE[10] ;
		size = 10 ;
	} ;

	ArrayList(LinkList<TYPE>& llist) {
		list = NULL ;
		size = 0 ;
		Init(llist) ;
	} ;

	~ArrayList() {
		delete [] list ;
	} ;

	void Init(LinkList<TYPE>& llist) {
		int i ;

		if(list) {
			delete [] list ;
		}
		size = llist.size ;
		list = new TYPE[size] ;
		if(size > 0) {
			llist.SeekHead() ;
			for(i = 0; i < size; i++) {
				list[i] = llist.Data() ;
				llist.Next() ;
			}
		}
	} ;

	inline TYPE& operator[](int index) {
		TYPE *plist ;
		int i ;

		if(index < 0 || size == 0) return list[0] ;

		if(index >= size) {
			plist = list ;
			if(index < size * 2)
				list = new TYPE[size * 2] ;
			else
				list = new TYPE[(index+1) * 2] ;
			for(i = 0; i < size; i++) {
				list[i] = plist[i] ;
			}
			delete [] plist ;
			if(index < size * 2)
				size = size * 2 ;
			else
				size = (index+1) * 2 ;
			return list[index] ;
		}
		return list[index] ;
	} ;

	inline void Sort(int n, int (*compare)(TYPE *, TYPE *)) {

		qsort(list, n, sizeof(TYPE *), (int (*)(const void *, const void *)) compare) ;
	} ;

	inline int BinarySearch(TYPE *c, int s, int e, int (*compare)(TYPE *, TYPE *)) {

		int m ;
		int cr ;
		int ns, ne ;

		if(s > e) return -1 ;

		ns = s ;
		ne = e ;
		m = (ns + ne) / 2 ;		
		cr = compare(c, &list[m]) ;
		while(ns < ne && cr) {
			if(cr < 0) {
				ne = m - 1 ;
			}
			if(cr > 0) {
				ns = m + 1 ;
			}
			m = (ns + ne) / 2 ;
			cr = compare(c, &list[m]) ;
		}
		if(!cr) return m ;
		return -1 ;
	}

	inline int BinarySearch(TYPE *c, int n, int (*compare)(TYPE *, TYPE *)) {

		return BinarySearch(c, 0, n - 1, compare) ;
	}

	inline int BinarySearch(TYPE c, int n, int (*compare)(TYPE *, TYPE *)) {

		return BinarySearch(&c, 0, n - 1, compare) ;
	}
} ;

#define PreDefine_LinkList(TYPE)\
	template class LinkListNode<TYPE> ; \
	template class LinkList<TYPE> ;

#endif
