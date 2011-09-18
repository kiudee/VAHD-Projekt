#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#undef new // Workaround: Macro "new" collides with definition from <vector>
#include <vector>
#undef new
#undef delete // Workaround: Macro "delete" collides with definition from <memory>
#include <memory>
#undef delete

#include <iostream>
#include <fstream>
#include "Subjects1-6.h"

// Workaround: Macro "new" collides with definition from <vector>
#define new(subject,object) \
    _create((Subject *) this, (Subject *) new subject((Object* &) object))
// Workaround: Macro "delete" collides with definition from <memory>
#define delete(subject) \
    _kill((Subject *) this, subject)

#include "Objects.h"
#include "Node.h"
#include "Hash.h"

SubjectType(Supervisor)
{
protected:
    int count;
    int total;
    std::vector<Relay *> Nodes;
    std::vector<IdObj *> StartID;
    std::shared_ptr<std::ofstream> csvFile;

public:
    FirstAction(Supervisor, Init)

    Action Init(NumObj * num);
    Action SetLink(IdPair * id);
    Action Wakeup(NumObj * num);
    Action RemoveRealChild(IdObj * ido);
};

#endif  // SUPERVISOR_H_
