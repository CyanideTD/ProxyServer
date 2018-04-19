#include "queue_t.h"

// template <typename T>
// CQueue<T>::CQueue()
// {
//     maxNodes = 0;
// }

// template <typename T>
// CQueue<T>::~CQueue()
// {

// }

// template<typename T>
// int CQueue<T>::Init(int maxNode)
// {
//     maxNodes = maxNode;
//     int result = 0;
//     result = sem_init(&m_semExist, 0, 0);
//     if (result == -1)
//     {
//         return -1;
//     }
//     result = sem_init(&m_semEmpty, 0, maxNode);
//     if (result == -1)
//     {
//         return -1;
//     }
    
//     result = pthread_mutex_init(&m_mtxQue, NULL);
    
//     return result;
// }

// template <typename T>
// int CQueue<T>::Uninit()
// {
//     while (!m_queue.empty())
//     {
//         T node = 0;
//         node = m_queue.top();
//         m_queue.pop();
//         delete node;
//     }
// }

// template<typename T>
// int CQueue<T>::WaitTillPush(const T& node)
// {
//     int flag = 0;
//     while (1)
//     {
//         flag = sem_wait(&m_semEmpty);
//         if (flag == -1)
//         {
//             if (errno == EINTR)
//             {
//                 continue;
//             }
//             else
//             {
//                 return flag;
//             }
//         }
//         else
//         {
//             break;
//         }
//     }

//     flag = pthread_mutex_lock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }

//     m_queue.push(node);
//     flag = sem_post(&m_semExist);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     flag = pthread_mutex_unlock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     return 0;
// }

// template<typename T>
// int CQueue<T>::WaitTillPop(T*& node)
// {
//     int flag = 0;
//     while (1)
//     {
//         flag = sem_wait(&m_semExist);
//         if (flag == -1)
//         {
//             if (errno == EINTR)
//             {
//                 continue;
//             }
//             else
//             {
//                 return -1;
//             }
//         }
//         else
//         {
//             break;
//         }
//     }

//     flag = pthread_mutex_lock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     *node = m_queue.top();
//     m_queue.pop();
//     flag = sem_post(&m_semEmpty);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     flag = pthread_mutex_unlock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     return 0;
// }

// template<typename T>
// int CQueue<T>::WaitTimePush(const T& node, unsigned int usec)
// {
//     int flag = 0;
//     struct timespec ts;
//     if (usec >= 1000000)
//     {
//         ts.tv_sec = usec / 1000000;
//         ts.tv_nsec = (usec % 1000000) * 1000;
//     }
//     else
//     {
//         ts.tv_nsec = usec * 1000;
//     }

//     while (1)
//     {
//         flag = sem_timedwait(&m_semEmpty, &ts);
//         if (flag == -1)
//         {
//             if (errno == EINTR)
//             {
//                 continue;
//             }
//             else
//             {
//                 return -1;
//             }
//         }
//         else
//         {
//             break;
//         }
//     }

//     flag = pthread_mutex_lock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }

//     m_queue.push(*node);
//     flag = sem_post(&m_semExist);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     flag = pthread_mutex_unlock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     return 0;
// }

// template<typename T>
// int CQueue<T>::WaitTimePop(T*& node, unsigned int usec)
// {
//     int flag = 0;
//     struct timespec ts;
//     if (usec >= 1000000)
//     {
//         ts.tv_sec = usec / 1000000;
//         ts.tv_nsec = (usec % 1000000) * 1000;
//     }
//     else
//     {
//         ts.tv_nsec = usec * 1000;
//     }

//     while (1)
//     {
//         flag = sem_timedwait(&m_semExist, &ts);
//         if (flag == -1)
//         {
//             if (errno == EINTR)
//             {
//                 continue;
//             }
//             else
//             {
//                 return -1;
//             }
//         }
//         else
//         {
//             break;
//         }
//     }

//     flag = pthread_mutex_lock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }

//     *node = m_queue.top();
//     m_queue.pop();
//     flag = sem_post(&m_semEmpty);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     flag = pthread_mutex_unlock(&m_mtxQue);
//     if (flag == -1)
//     {
//         pthread_mutex_unlock(&m_mtxQue);
//         return -1;
//     }
//     return 0;
// }

// template<typename T>
// bool CQueue<T>:: IsEmpty()
// {
//     return m_queue.empty();
// }

// template<typename T>
// unsigned int CQueue<T>::GetSize()
// {
//     return m_queue.size();
// }

// template<typename T>
// unsigned int CQueue<T>::GetCapacity()
// {
//     return maxNodes;
// }