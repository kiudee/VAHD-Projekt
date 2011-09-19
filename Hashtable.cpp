#include <math.h>
#include <cstdint>
#include <iostream>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Supervisor.h"
#include "Node.h"


int main()
{
    NumObj *numSubjects = new NumObj(500);
    runSubjects(Supervisor, numSubjects, 15000);
    sys_pause();
}

