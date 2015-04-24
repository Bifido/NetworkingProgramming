//
// Created by Cristian Marastoni on 23/04/15.
//

#ifndef NETWORK_DEBUG_H
#define NETWORK_DEBUG_H


class Debug {
public:
    static void Log(const char *ctx, const char *fmt, ...);
    static void Error(const char *ctx, const char *fmt, ...);
};


#endif //NETWORK_DEBUG_H
