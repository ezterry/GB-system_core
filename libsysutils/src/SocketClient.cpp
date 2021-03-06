#include <alloca.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <string.h>

#define LOG_TAG "SocketClient"
#include <cutils/log.h>

#include <sysutils/SocketClient.h>

SocketClient::SocketClient(int socket)
        : mSocket(socket)
        , mPid(-1)
        , mUid(-1)
        , mGid(-1)
{
    pthread_mutex_init(&mWriteMutex, NULL);

    struct ucred creds;
    socklen_t szCreds = sizeof(creds);
    memset(&creds, 0, szCreds);

    int err = getsockopt(socket, SOL_SOCKET, SO_PEERCRED, &creds, &szCreds);
    if (err == 0) {
        mPid = creds.pid;
        mUid = creds.uid;
        mGid = creds.gid;
    }
}

int SocketClient::sendMsg(int code, const char *msg, bool addErrno) {
    char *buf;
    const char* arg;
    const char* fmt;
    char tmp[1];
    int  len;

    if (addErrno) {
        fmt = "%.3d %s (%s)";
        arg = strerror(errno);
    } else {
        fmt = "%.3d %s";
        arg = NULL;
    }
    /* Measure length of required buffer */
    len = snprintf(tmp, sizeof tmp, fmt, code, msg, arg);
    /* Allocate in the stack, then write to it */
    buf = (char*)alloca(len+1);
    snprintf(buf, len+1, fmt, code, msg, arg);
    /* Send the zero-terminated message */
    return sendMsg(buf);
}

int SocketClient::sendMsg(const char *msg) {
    if (mSocket < 0) {
        errno = EHOSTUNREACH;
        return -1;
    }

    // Send the message including null character
    int rc = 0;
    const char *p = msg;
    int brtw = strlen(msg) + 1;

    pthread_mutex_lock(&mWriteMutex);
    while (brtw > 0) {
        rc = write(mSocket, p, brtw);
        if (rc > 0) {
            p += rc;
            brtw -= rc;
            continue;
        }

        if (rc < 0 && errno == EINTR)
            continue;

        pthread_mutex_unlock(&mWriteMutex);
        if (rc == 0) {
            SLOGW("0 length write :(");
            errno = EIO;
        } else {
            SLOGW("write error (%s)", strerror(errno));
        }
        return -1;
    }
    pthread_mutex_unlock(&mWriteMutex);
    return 0;
}
