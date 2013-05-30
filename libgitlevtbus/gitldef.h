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
#define SINGLETON_PATTERN_DECLARE(classname)\
    public: \
        static classname* getInstance() { QMutexLocker cLocker(&m_cGetInstanceMutex); if(m_instance==NULL) m_instance=new classname(); return m_instance; } \
    private: \
        static classname* m_instance; \
        static QMutex m_cGetInstanceMutex;

#define SINGLETON_PATTERN_IMPLIMENT(classname)\
    classname* classname::m_instance = NULL; \
    QMutex classname::m_cGetInstanceMutex;


/// CLIP c BETWEEN a AND b
#define VALUE_CLIP(a,b,c) ( ((c)>(b))?((c)=(b)):((c)<(a))?((c)=(a)):(c) )





#endif
