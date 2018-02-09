////#include "LinkedList.h"
//
//template<class T>
//LinkedList<T>::LinkedList(void)
//{
//	pHead = NULL;
//	pTail = NULL;
//	nElements = 0;
//}
//
//template<class T>
//LinkedList<T>::~LinkedList(void)
//{
//	for(int i = 0; i < nElements; i++)
//	{
//		pop_back();
//	}
//
//	pHead = NULL;
//	pTail = NULL;
//}
//
//template<class T>
//void LinkedList<T>::push_back(T obj)
//{
//	if(pHead == NULL)
//	{
//		pHead = new Node<T>(obj);
//		pHead->pLastNode = pHead;
//		pTail = pHead;
//		nElements++;
//	}
//	else
//	{
//		pTail->pNextNode = new Node<T>(obj);
//		pTail->pNextNode->pLastNode = pTail;
//		pTail = pTail->pNextNode;
//		nElements++;
//	}
//}
//
//template<class T>
//void LinkedList<T>::pop_back()
//{
//	if((pTail != NULL) && (pTail != pHead))
//	{
//		pTail = pTail->pLastNode;
//		delete pTail->pNextNode;
//		pTail->pNextNode = NULL;
//		nElements--;
//	}
//	else if((pTail != NULL) && (pTail == pHead))
//	{
//		delete pHead;
//		pTail = NULL;
//		pHead = NULL;
//		nElements = 0;
//	}
//}
//
//template<class T>
//void LinkedList<T>::clear()
//{
//	while(pHead != NULL)
//	{
//		pop_back();
//	}
//
//	nElements = 0;
//}
//
//template<class T>
//int LinkedList<T>::size()
//{
//	return nElements;
//}
//
//template<class T>
//T LinkedList<T>::operator[](const unsigned int index)
//{
//	int i = 0;
//	Node<T>* pSeek = pHead;
//
//	if(index < nElements)
//	{
//		while(i != index)
//		{
//			pSeek = pSeek->pNextNode;
//			i++;
//		}
//		
//		return pSeek;
//	}
//	else
//	{
//		return NULL;
//	}
//}
