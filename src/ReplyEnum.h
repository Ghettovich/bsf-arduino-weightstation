#ifndef BSF_ARDUINO_WEIGHTSTATION_REPLYENUM_H
#define BSF_ARDUINO_WEIGHTSTATION_REPLYENUM_H

enum ReplyWithCode { FULL_STATE_RPLY, RELAY_STATE_RPLY, DETECTION_STATE_RPLY, EMPTY };

enum ReplyWithCode identifyReplyCode(int replyStateCode);

enum ReplyWithCode identifyReplyCode(int replyStateCode) {

    switch (replyStateCode) {
        case FULL_STATE_RPLY:
            return FULL_STATE_RPLY;
        case RELAY_STATE_RPLY:
            return RELAY_STATE_RPLY;
        case DETECTION_STATE_RPLY:
            return DETECTION_STATE_RPLY;
        case EMPTY:
            return EMPTY;
        default:
            return FULL_STATE_RPLY;

    }
}

#endif //BSF_ARDUINO_WEIGHTSTATION_REPLYENUM_H