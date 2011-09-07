#include <math.h>
#include <iostream>
#include <cstdint>
#include "Subjects1-6.h"
#include "Objects.h"
#include "Supervisor.h"
#include "Node.h"


int main()
{
    NumObj *numSubjects = new NumObj(10);
    runSubjects(Supervisor,numSubjects,500);
    sys_pause();
}

