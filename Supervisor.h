#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#include "Hash.h"

SubjectType(Supervisor)
{
protected:
    int count;
    int total;
    Relay** ListNode;
    IdObj** StartID;

public:
    FirstAction(Supervisor, Init)
    Action Init(NumObj *num);
    Action SetLink(IdPair *id);
    Action Wakeup(NumObj *num);
};

#endif  // SUPERVISOR_H_
