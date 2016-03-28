/*
 *
 * 	T* alloc()
 *  void free(T*)
 *  void free_all()
 *
 */

#ifndef _MY_FSA_HPP
#define _MY_FSA_HPP

namespace my {

	template<class T>
	class fsa {
		struct fsaElem {
				T UserType;
				fsaElem *pNext;
		};
	public:
			fsa( unsigned int max_elements ) {
				m_maxElements = max_elements;
				m_pMemory = new fsaElem[ max_elements ];

				m_pFirstFree = m_pMemory;
				moved = false;

				fsaElem *elem = m_pMemory;
				for(int i=0; i < max_elements; i++ ) {
					elem->pNext = elem+1;
					elem++;
				}
				(elem-1)->pNext = nullptr;
			}

			fsa ( fsa&& f ) : m_pMemory(f.m_pMemory){
				moved = false;
				f.moved = true;
			};

			~fsa() {
				if(!moved) {
					delete [] m_pMemory;
				}
			}


			T* alloc() {
				if(m_pFirstFree) {
					T* elem = reinterpret_cast<T*>(m_pFirstFree);
					m_pFirstFree = m_pFirstFree->pNext;
					return elem;
				} else {
					return nullptr;
				}
			}

			void free( T* elem ) {
				reinterpret_cast<fsaElem*>(elem)->pNext = m_pFirstFree;
				m_pFirstFree = reinterpret_cast<fsaElem*>(elem);
			}

			void free_all() {
				fsaElem* elem = m_pMemory;
				for(int i=0; i < m_maxElements; i++ ) {
					elem->pNext = elem+1;
					elem++;
				}
				(elem-1)->pNext = nullptr;
			}

			void* GetMemLoc() const {
				return (void*)m_pMemory;
			}

	private:
			fsaElem *m_pFirstFree;
			fsaElem *m_pMemory;
			unsigned int m_maxElements;
			bool moved;
	};
};

#endif

