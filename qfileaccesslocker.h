#ifndef FILEACCESSLOCKER_H
#define FILEACCESSLOCKER_H

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

        auto lock = m_locks.value (fileName);
        locker.unlock ();
        lock->lockForWrite();
    }

    inline void lockForRead (const QString& fileName)
    {
        QMutexLocker locker (&m_mutex);

        if (!m_locks.contains (fileName))
            m_locks.insert (fileName, QSharedPointer<QReadWriteLock> (new QReadWriteLock));

        auto lock = m_locks.value (fileName);
        locker.unlock ();
        lock->lockForRead();
    }

    inline void unlock (const QString& fileName)
    {
        QMutexLocker locker (&m_mutex);

        if (!m_locks.contains (fileName))
            return;

        auto lock = m_locks.value (fileName);
        locker.unlock ();
        lock->unlock();
    }

private:
    Q_DISABLE_COPY_MOVE (QFileAccessLocker)
    inline QFileAccessLocker() = default;

private:
    QHash<QString, QSharedPointer<QReadWriteLock>> m_locks;
    QMutex                                         m_mutex;
};

class QFileWriteLockGuard
{
public:
    inline QFileWriteLockGuard (const QString& fileName) : m_fileName (fileName)
    {
        QFileAccessLocker::instance().lockForWrite (m_fileName);
    }

    inline ~QFileWriteLockGuard()
    {
        if (!m_fileName.isEmpty())
            unlock();
    }

    inline void unlock()
    {
        QFileAccessLocker::instance().unlock (m_fileName);
        m_fileName.clear();
    }

private:
    Q_DISABLE_COPY_MOVE (QFileWriteLockGuard)
    QString m_fileName;
};

class QFileReadLockGuard
{
public:
    inline QFileReadLockGuard (const QString& fileName) : m_fileName (fileName)
    {
        QFileAccessLocker::instance().lockForRead (m_fileName);
    }

    inline ~QFileReadLockGuard()
    {
        if (!m_fileName.isEmpty())
            unlock();
    }

    inline void unlock()
    {
        QFileAccessLocker::instance().unlock (m_fileName);
        m_fileName.clear();
    }

private:
    Q_DISABLE_COPY_MOVE (QFileReadLockGuard)
    QString m_fileName;
};

#endif // FILEACCESSLOCKER_H
