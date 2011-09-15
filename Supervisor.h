#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#undef new // Workaround: Macro "new" collides with definition from <vector>
#include <vector>
#undef new

#include <fstream>
#include <sstream>
#include <iostream>
#include "Subjects1-6.h"

// Workaround: Macro "new" collides with definition from <vector>
#define new(subject,object) \
    _create((Subject *) this, (Subject *) new subject((Object* &) object))

#include "Objects.h"
#include "Node.h"
#include "Hash.h"



SubjectType(Supervisor)
{
protected:
    int count;
    int total;
    std::vector<NodeRelay *> Nodes;
    std::vector<IdObj *> StartID;

    void freezeGraph();
    void unfreezeGraph();
    std::string Node2GDL(IdObj *id);
    std::string Edge2GDL(IdObj *src, IdObj *target);
public:
    FirstAction(Supervisor, Init)

    Action Init(NumObj * num);
    Action SetLink(IdPair * id);
    Action Wakeup(NumObj * num);
};

#endif  // SUPERVISOR_H_
