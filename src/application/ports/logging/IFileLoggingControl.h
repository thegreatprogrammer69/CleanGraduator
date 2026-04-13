#ifndef CLEANGRADUATOR_IFILELOGGINGCONTROL_H
#define CLEANGRADUATOR_IFILELOGGINGCONTROL_H

namespace application::ports {
    struct IFileLoggingControl {
        virtual ~IFileLoggingControl() = default;

        virtual void setFileLoggingEnabled(bool enabled) = 0;
        virtual bool fileLoggingEnabled() const = 0;
    };
}

#endif //CLEANGRADUATOR_IFILELOGGINGCONTROL_H
