#ifndef GITLDEF_H
#define GITLDEF_H
#include <QMutex>
#include <QMutexLocker>


/// AUTO GETTER AND SETTER FOR CLASS FIELDS
#define ADD_CLASS_FIELD(type, name, getter, setter) \
    public: \
        type& getter() { return m_##name; } \
        void setter(type name) { m_##name = name; } \
    private: \
        type m_##name;

#define ADD_CLASS_FIELD_NOSETTER(type, name, getter) \
    public: \
        type& getter() { return m_##name; } \
    private: \
        type m_##name;

#define ADD_CLASS_FIELD_PRIVATE(type, name ) \
    private: \
        type m_##name;


/// SINGLETON DESIGN PATTERN (Thread Safe)
//#define SINGLETON_PATTERN_DECLARE(classname)\
//    public: \
//        static classname* getInstance() { QMutexLocker cLocker(&m_cGetInstanceMutex); if(m_instance==NULL) m_instance=new classname(); return m_instance; } \
//    private: \
//        static classname* m_instance; \
//        static QMutex m_cGetInstanceMutex;

//#define SINGLETON_PATTERN_IMPLIMENT(classname)\
//    classname* classname::m_instance = NULL; \
//    QMutex classname::m_cGetInstanceMutex;


/// CLIP c BETWEEN a AND b
#define VALUE_CLIP(a,b,c) ( ((c)>(b))?((c)=(b)):((c)<(a))?((c)=(a)):(c) )



template <class T>
class GitlSingleton
{
public:
       static inline T* getInstance();

private:
       GitlSingleton(void){}
       ~GitlSingleton(void){}
       GitlSingleton(const GitlSingleton&){}
       GitlSingleton & operator= (const GitlSingleton &){}

       static T* m_instance; \
       static QMutex m_cGetInstanceMutex;
};

template <class T>
GitlSingleton<T>* GitlSingleton<T>::m_instance = NULL;

template <class T>
QMutex GitlSingleton<T>::m_cGetInstanceMutex;

template <class T>
inline T* GitlSingleton<T>::getInstance()
{
     QMutexLocker cLocker(&m_cGetInstanceMutex);
     if(m_instance==NULL)
         m_instance=new T();
     return m_instance;
}

//Class that will implement the singleton mode,
//must use the macro in it's delare file
#define SINGLETON_PATTERN_DECLARE( type ) \
       //friend class auto_ptr< type >;\
       friend class GitlSingleton< type >;



#endif
