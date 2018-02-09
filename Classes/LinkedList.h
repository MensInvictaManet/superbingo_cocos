#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include "MemoryTracker/MemoryTracker.h"

/// **************************************************
/// Doubly Linked-List implementation designed to
/// replace std::vector usage in SuperBingo.
/// It uses the same function names to limit code
/// changes.
/// **************************************************

template <class T>
class LLNode
{
	//template <class T>
	//friend class LinkedList;

	//private:
	public:
		LLNode(T obj) { data = obj; pLastNode = NULL; pNextNode = NULL; };
		~LLNode() { };
		inline const LLNode<T>& operator=(const LLNode<T>& rhs) { data = rhs.data;	return this; }
		inline const LLNode<T>& operator=(const T& rhs)		{ data = rhs;		return this; }

		T	  data;
		LLNode* pLastNode;
		LLNode* pNextNode;
};

template<class T>
class LLNodeFwdIterator
{
	public:
		LLNodeFwdIterator(LLNode<T>* pos = NULL) : _pos(pos) {}
 
		bool operator==(const LLNodeFwdIterator& rhs) const {return (_pos == rhs._pos);}
		bool operator!=(const LLNodeFwdIterator& rhs) const {return !operator==(rhs);}
                          
		LLNodeFwdIterator& operator++()
		{
			if (_pos != NULL)
				_pos = _pos->pNextNode;
 
			return *this;
		}
 
		LLNodeFwdIterator operator++(int)
		{
			LLNodeFwdIterator retval = *this;
 
			operator++();
 
			return retval;
		}
 
		T& operator*() {return _pos->data;}
		T& operator->() {return _pos->data;}
 
	private:
		LLNode<T>* _pos;
};

template <class T>
class LinkedList
{	
	private:
		LLNode<T>* m_pHead;
		LLNode<T>* m_pTail;
		unsigned int m_nElements;

	public:
		inline LinkedList();
		inline ~LinkedList();
		inline void push_back(T obj);
		inline void pop_back();
		inline void delete_back();
		inline void clear();
		inline void clear_and_delete();
		inline bool empty() const;
		inline void resize_down(int nSize);
		inline void setVal(int index, T val);
		inline void swap_at_indices(unsigned int a, unsigned int b);
		inline T operator[](const unsigned int index);
		inline const T operator[](const unsigned int index) const;
		//inline const LinkedList<T>& operator=(const LinkedList<T>& rhs);
		inline const LLNode<T>& operator=(const T& rhs);
		inline const LLNode<T>& operator=(const LLNode<T>& rhs);
		//inline const T& operator=(const T& rhs);

		inline unsigned int size() const { return m_nElements; }
		inline T first()  { return (m_pHead == NULL) ? NULL : m_pHead->data; }
		inline T back()	  { return (m_pTail == NULL) ? NULL : m_pTail->data; } 

public:
		typedef LLNodeFwdIterator<T> iterator;
		iterator begin()     { return m_pHead; }
		iterator end()       { return NULL; }
};

/// <summary>
/// Constructor
/// </summary>
template<class T>
inline LinkedList<T>::LinkedList(void)
{
	m_pHead = NULL;
	m_pTail = NULL;
	m_nElements = 0;
}

/// <summary>
/// Destructor
/// </summary>
template<class T>
inline LinkedList<T>::~LinkedList(void)
{
	while (!empty())
	{
		pop_back();
	}

	m_pHead = NULL;
	m_pTail = NULL;
}

/// <summary>
/// Push a new object into the list.
/// </summary>
template<class T>
inline void LinkedList<T>::push_back(T obj)
{
	if (m_pHead == NULL)
	{
		m_pHead = new LLNode<T>(obj);
		TrackAllocate("LinkedList Node", sizeof(LLNode<T>));
		m_pHead->pLastNode = m_pHead;
		m_pTail = m_pHead;
		m_nElements++;
	}
	else
	{
		m_pTail->pNextNode = new LLNode<T>(obj);
		TrackAllocate("LinkedList Node", sizeof(LLNode<T>));
		m_pTail->pNextNode->pLastNode = m_pTail;
		m_pTail = m_pTail->pNextNode;
		m_nElements++;
	}
}

/// <summary>
/// Pop the last object in the list.
/// </summary>
template<class T>
inline void LinkedList<T>::pop_back()
{
	if ((m_pTail != NULL) && (m_pTail != m_pHead))
	{
		m_pTail = m_pTail->pLastNode;
		//delete m_pTail->pNextNode;
		m_pTail->pNextNode = NULL;
		m_nElements--;
	}
	else if ((m_pTail != NULL) && (m_pTail == m_pHead))
	{
		//delete m_pHead;
		m_pTail = NULL;
		m_pHead = NULL;
		m_nElements = 0;
	}
}

/// <summary>
/// Delete the last object in the list.
/// </summary>
template<class T>
inline void LinkedList<T>::delete_back()
{
	if ((m_pTail != NULL) && (m_pTail != m_pHead))
	{
		m_pTail = m_pTail->pLastNode;
		TrackDeallocate("LinkedList Node", sizeof(LLNode<T>));
		delete m_pTail->pNextNode;
		m_pTail->pNextNode = NULL;
		m_nElements--;
	}
	else if ((m_pTail != NULL) && (m_pTail == m_pHead))
	{
		TrackDeallocate("LinkedList Node", sizeof(LLNode<T>));
		delete m_pHead;
		m_pTail = NULL;
		m_pHead = NULL;
		m_nElements = 0;
	}
}

/// <summary>
/// Clear the list of all objects
/// </summary>
template<class T>
inline void LinkedList<T>::clear()
{
	while (m_nElements != 0)
	{
		if ((m_pTail != NULL) && (m_pTail != m_pHead))
		{
			m_pTail = m_pTail->pLastNode;
			//delete m_pTail->pNextNode;
			//m_pTail->pNextNode = NULL;
			m_nElements--;
		}
		else if ((m_pTail != NULL) && (m_pTail == m_pHead))
		{
			//delete m_pHead;
			m_pTail = NULL;
			m_pHead = NULL;
			m_nElements = 0;
		}
	}

	m_nElements = 0;
}

/// <summary>
/// Clear the list of all objects
/// </summary>
template<class T>
inline void LinkedList<T>::clear_and_delete()
{
	while (m_pHead != NULL)
	{
		delete_back();
	}

	m_nElements = 0;
}

/// <summary>
/// Checks if the list is empty.
/// </summary>
template<class T>
inline bool LinkedList<T>::empty() const
{
	if (m_nElements == 0)
	{
		return true;
	}

	return false;
}

template<class T>
inline void LinkedList<T>::resize_down(int nSize)
{
	while (m_nElements > nSize)
	{
		pop_back();
	}
}

template<class T>
inline void LinkedList<T>::setVal(int index, T val)
{
	int i = 0;
	LLNode<T>* pSeek = m_pHead;

	if (index < m_nElements)
	{
		while (i != index)
		{
			pSeek = pSeek->pNextNode;
			i++;
		}

		pSeek->data = val;
	}
}

template<class T>
inline void LinkedList<T>::swap_at_indices(unsigned int a, unsigned int b)
{
	unsigned int i = 0;
	bool updatedHead = false;
	bool updatedTail = false;
	LLNode<T>* pSeekA = m_pHead;
	LLNode<T>* pSeekB = m_pHead;
	LLNode<T>* pTemp;
	LLNode<T>* pTemp2;

	if ((a < m_nElements) && (b < m_nElements))
	{
		while (i != a)
		{
			pSeekA = pSeekA->pNextNode;
			i++;
		}

		i = 0;

		while (i != b)
		{
			pSeekB = pSeekB->pNextNode;
			i++;
		}

		// Update the references for nodes surrounding A
		if (pSeekA != m_pHead)
		{
			pTemp = pSeekA->pLastNode;
			pTemp->pNextNode = pSeekB;
		}

		if (pSeekA != m_pTail)
		{
			pTemp = pSeekA->pNextNode;
			pTemp->pLastNode = pSeekB;
		}

		// Update the references for nodes surrounding B
		if (pSeekB != m_pHead)
		{
			pTemp = pSeekB->pLastNode;
			pTemp->pNextNode = pSeekA;
		}
		
		if (pSeekB != m_pTail)
		{
			pTemp = pSeekB->pNextNode;
			pTemp->pLastNode = pSeekA;
		}

		// Update the head and tail if necessary
		if (pSeekA == m_pHead)
		{
			m_pHead = pSeekB;
			updatedHead = true;
		}
		if (pSeekA == m_pTail)
		{
			m_pTail = pSeekB;
			updatedTail = true;
		}
		if ((pSeekB == m_pHead) && !updatedHead)
		{
			m_pHead = pSeekA;
		}
		if ((pSeekB == m_pTail) && !updatedTail)
		{
			m_pTail = pSeekA;
		}

		// Update the references for nodes A and B
		pTemp = pSeekA->pLastNode;
		pTemp2 = pSeekA->pNextNode;
		pSeekA->pLastNode = pSeekB->pLastNode;
		pSeekA->pNextNode = pSeekB->pNextNode;
		pSeekB->pLastNode = pTemp;
		pSeekB->pNextNode = pTemp2;
	}
}

/// <summary>
/// Access an element at a specific index.
/// </summary>
template<class T>
inline T LinkedList<T>::operator[](const unsigned int index)
{
	unsigned int i = 0;
	LLNode<T>* pSeek = m_pHead;

	if (index < m_nElements)
	{
		while (i != index)
		{
			pSeek = pSeek->pNextNode;
			i++;
		}
		
		return pSeek->data;
	}
	else
	{
		return NULL;
	}
}

/// <summary>
/// Access an element at a specific index.
/// </summary>
template<class T>
inline const T LinkedList<T>::operator[](const unsigned int index) const
{
	unsigned int i = 0;
	LLNode<T>* pSeek = m_pHead;

	if (index < m_nElements)
	{
		while (i != index)
		{
			pSeek = pSeek->pNextNode;
			i++;
		}
		
		return pSeek->data;
	}
	else
	{
		return NULL;
	}
}

template<class T>
inline const LLNode<T>& LinkedList<T>::operator=(const T& rhs)
{
	this->data = rhs;
}

template<class T>
inline const LLNode<T>& LinkedList<T>::operator=(const LLNode<T>& rhs)
{
	this->data = rhs->data;
}

template <class T>
class ArrayList
{	
	private:
		LLNode<T>* m_pHead;
		LLNode<T>* m_pTail;
		unsigned int m_nElements;

	public:
		inline ArrayList();
		inline ~ArrayList();
		inline void push_back(T obj);
		inline void pop_back();
		inline void delete_back();
		inline void clear();
		inline void clear_and_delete();
		inline bool empty();
		inline void resize_down(int nSize);
		inline void setVal(int index, T val);
		inline void swap_at_indices(unsigned int a, unsigned int b);
		inline T operator[](const unsigned int index);
		inline const LLNode<T>& operator=(const T& rhs);
		inline const LLNode<T>& operator=(const LLNode<T>& rhs);

		inline unsigned int size() { return m_nElements; }
		inline T back()	  { return m_pTail->data; }
};

#endif // _LIST_H_