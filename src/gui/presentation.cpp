#include "guiPresentation.h"

#if defined(__ANDROID__)
#include <SDL_system.h>
#include <jni.h>
#endif

namespace holdem::gui {

void requestPresentation(Presentation presentation, int preferredWidth, int preferredHeight) {
    (void)preferredWidth;
    (void)preferredHeight;

#if defined(__ANDROID__)
    JNIEnv* env = static_cast<JNIEnv*>(SDL_AndroidGetJNIEnv());
    jobject activity = static_cast<jobject>(SDL_AndroidGetActivity());
    if (env == nullptr || activity == nullptr) {
        return;
    }

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass == nullptr) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID method = env->GetMethodID(activityClass, "requestGamePresentation", "(Z)V");
    if (method != nullptr && !env->ExceptionCheck()) {
        env->CallVoidMethod(activity, method, presentation == Presentation::Game ? JNI_TRUE : JNI_FALSE);
    }
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }

    env->DeleteLocalRef(activityClass);
    env->DeleteLocalRef(activity);
#else
    (void)presentation;
#endif
}

} // namespace holdem::gui
