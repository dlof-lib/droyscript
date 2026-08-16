// JNI bridge: com.dlof.droyscript.engine.DroyEngine <-> droy::run()
#include <jni.h>
#include <string>
#include "droy/interpreter.hpp"

extern "C" JNIEXPORT jstring JNICALL
Java_com_dlof_droyscript_engine_DroyEngine_nativeRun(JNIEnv* env, jobject /*thiz*/, jstring jsource) {
    const char* src = env->GetStringUTFChars(jsource, nullptr);
    std::string source(src);
    env->ReleaseStringUTFChars(jsource, src);

    std::string result;
    try {
        result = droy::run(source);
    } catch (const droy::DroyError& e) {
        result = std::string("ERROR: ") + e.what();
    } catch (const std::exception& e) {
        result = std::string("ERROR: ") + e.what();
    }
    return env->NewStringUTF(result.c_str());
}
