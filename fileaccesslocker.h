#ifndef FILEACCESSLOCKER_H
#define FILEACCESSLOCKER_H

#include <QObject>
#include <QHash>
#include <QReadWriteLock>
#include <QSharedPointer>
#include <QMutex>
#include <QMutexLocker>

//! defined for Qt versions prior to 5.13
#ifndef Q_DISABLE_COPY_MOVE
#define Q_DISABLE_COPY_MOVE(Class)\
    Class(const Class &) Q_DECL_EQ_DELETE;\
    Class &operator=(const Class &) Q_DECL_EQ_DELETE;\
    Class(Class &&) Q_DECL_EQ_DELETE;\
    Class &operator=(Class &&) Q_DECL_EQ_DELETE;
#endif

//! Singleton class used for file access synchronization
//! It is designed to extend QReadWriteLock's functionality to manage multiple files access synchronization between multiple reading and writing threads.
//! Writing threads have higher priority, so they don't get blocked forever by reading threads (see QReadWriteLock documentation).
class QFileAccessLocker
{
public:
    inline static QFileAccessLocker& instance ()
    {
        static QFileAccessLocker instance;
        return instance;
    }

    inline void lockForWrite (const QString& fileName)
    {
        QMutexLocker locker (&m_mutex);

        if (!m_locks.contains (fileName))
            m_locks.insert (fileName, QSharedPointer<QReadWriteLock> (new QReadWriteLock));

        m_locks.value (fileName)->lockForWrite();
    }

    inline void lockForRead (const QString& fileName)
    {
        QMutexLocker locker (&m_mutex);

        if (!m_locks.contains (fileName))
            m_locks.insert (fileName, QSharedPointer<QReadWriteLock> (new QReadWriteLock));

        m_locks.value (fileName)->lockForRead();
    }

    inline void unlock (const QString& fileName)
    {
        QMutexLocker locker (&m_mutex);

        if (!m_locks.contains (fileName))
            return;

        m_locks.value (fileName)->unlock();
    }

private:
    Q_DISABLE_COPY_MOVE (QFileAccessLocker)
    QFileAccessLocker() = default;

private:
    QHash<QString, QSharedPointer<QReadWriteLock>> m_locks;
    QMutex                                         m_mutex;
};

//! RAII-wrapper for write-lock with functionality similar to std::scoped_lock(lock_guard) or QMutexLocker
class QFileWriteLockGuard
{
public:
    inline QFileWriteLockGuard (const QString& fileName) : m_fileName (fileName)
    {
        QFileAccessLocker::instance().lockForWrite (m_fileName);
    }

    inline ~QFileWriteLockGuard()
    {
        QFileAccessLocker::instance().unlock (m_fileName);
    }

private:
    Q_DISABLE_COPY_MOVE (QFileWriteLockGuard)
    QString m_fileName;
};

//! RAII-wrapper for read-lock with functionality similar to std::scoped_lock(lock_guard) or QMutexLocker
class QFileReadLockGuard
{
public:
    inline QFileReadLockGuard (const QString& fileName) : m_fileName (fileName)
    {
        QFileAccessLocker::instance().lockForRead (m_fileName);
    }

    inline ~QFileReadLockGuard()
    {
        QFileAccessLocker::instance().unlock (m_fileName);
    }

private:
    Q_DISABLE_COPY_MOVE (QFileReadLockGuard)
    QString m_fileName;
};

#endif // FILEACCESSLOCKER_H
