#ifndef SUPERVISOR_H_
#define SUPERVISOR_H_

#undef new // Workaround: Macro "new" collides with definition from <vector>
#include <vector>
#undef new

#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include "Subjects1-6.h"

// Workaround: Macro "new" collides with definition from <vector>
#define new(subject,object) \
    _create((Subject *) this, (Subject *) new subject((Object* &) object))

#include "Objects.h"
#include "Node.h"
#include "Hash.h"

enum EdgeType { LEFT, RIGHT, EDGE1, EDGE0, LEFTRIGHT };

SubjectType(Supervisor)
{
protected:
    int count;
    int total;
    std::vector<NodeRelay *> Nodes;
    std::vector<IdObj *> StartID;

    std::vector<Subject *> Subjects;
    void printGraph();
    void freezeGraph();
    void unfreezeGraph();
    std::string Node2GDL(int id, double num, bool isReal);
    std::string Edge2GDL(int sourceid, int targetid, int edgetype);
public:
    FirstAction(Supervisor, Init)

    Action Init(NumObj * num);
    Action SetLink(IdPair * id);
    Action AddVirtuals(NodePair * np);
    Action Wakeup(NumObj * num);
};

#endif  // SUPERVISOR_H_
