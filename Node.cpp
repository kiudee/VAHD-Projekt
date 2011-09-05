#include "Subjects1-6.h"
#include "Node.h"

Action Node::Init(ConObj *con)
{
    // Unpack constructor object:
    id = con->position;
    isReal = con->isReal;
    delete con;

    // Initialize pointers and relays
    left = NULL;
    right = NULL;
    in = new Relay;
    // Connect to supervisor or real node:
    if (isReal) {
        IdPair *idp = new IdPair(new IdObj(id, new Identity(in)),
                         new IdObj(id, new Identity(in)));
        parent->call(Supervisor::SetLink, idp);
    } else {
        IdObj *ido = new IdObj(id, new Identity(in));
        parent->call(Node::RegisterChild, ido);
    }
}

Action Node::Wakeup(NumObj *num)
{
    // Periodically activate BuildDeBruijn:
    if (num->num == 0) {
        BuildDeBruijn(NULL);
    } else {
        num->num--;
        call(Node::Wakeup, num);
    }
}

void Node::checkDead(Relay *side) {
    if (side != NULL && outdeg(side->out) == 0) {
        delete side->out;
        delete side;
        side = NULL;
    }
}

void Node::checkValid() {
    IdObj *tempido;
    if (left != NULL && left->id > id) {
        tempido = new IdObj(left->id, extractIdentity(left->out));
        delete left;
        left = NULL;
        call(Node::BuildDeBruijn, tempido);
    }
    if (right != NULL && right->id < id) {
        tempido = new IdObj(right->id, extractIdentity(right->out));
        delete right;
        right = NULL;
        call(Node::BuildDeBruijn, tempido);
    }
}

Action Node::BuildDeBruijn(IdObj *id)
{
    if (isReal && ido == NULL) {
        // Probing:
        if (left != NULL) {
            // TODO: Probing to the left side for node0.
        }
        if (right != NULL) {
            // TODO: Probing to the right side for node1
        }
    }

    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(left);
    checkDead(right);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();


    /**
     * Simon:
     * 	würde das so machen:
     * 	BuildDeBruijn() hat keine Parameter und wird von WakeUp aufgerufen
     *
     * 	Action Node::BuildDeBruijn(IdObj *id){
     * 		BuildList(NULL); //ist dann nahezu identitsch mit Subj-List.cpp => BuildList
     * 		Probing(NULL);
     * 	}
     */

}

Action Node::Insert(NumObj *num)
{

}

Action Node::Delete(NumObj *key)
{

}

Action Node::LookUp(NumObj *key)
{

}

Action Node::Join(IdObj *id)
{

}

Action Node::Leave(IdObj *id)
{

}
/**
 * @author Simon
 */
Action Node::BuildList(IdObj *id){
    // Check if there are dead links from both sides:
    //   -> Delete if dead.
    checkDead(left);
    checkDead(right);

    // Check if both links are still valid:
    //   -> Call BuildDeBruijn if not.
    checkValid();
}
/**
 * @author Simon
 */
Action Node::Probing(IDObj *ido){
	if(isReal){
		if(ido==NULL){
			if(left==NULL){
				tempido = new IdObj(node0->id, new Identity(node0->in));
				left = new NodeRelay(tempido);
			}else{
				tempido = new IdObj(id, new Identity(in));
				left->out->call(Node::Probing, tempido);
			}
			if(right==NULL){
				tempido = new IdObj(node1->id, new Identity(node1->in));
				right = new NodeRelay(tempido);
			}else{
				tempido = new IdObj(id, new Identity(in));
				right->out->call(Node::Probing, tempido);
			}
		}
		else{
			if(ido->id=value/2){

			}
			else if(ido->id=1+value/2){

			}
		}


	}else{
		if(ido!=NULL){
			if(ido->value < value){ //probe kam von rechts
				if(ido->value < value/2){ //v.0 wurde ueberlaufen
					//erzeuge kante von v nach v.0
					//
					/*
					 * Also meine Idee war jetzt an dieser Stelle
					 * auszunutzen, dass man weiss, woher das Probing kam und
					 * v <- probing(v.0) aufzurufen. Leider müsste man dafür ein
					 * bestehendes Relay missbrauchen und ausserdem kennen wir hier
					 * an dieser Stelle nicht die Identity vom v.0-Relay.
					 * Das heisst wir muessen wieder zu v zurückrouten, aber das
					 * geht nicht innerhalb von Probing(), da wir in die andere
					 * Richtung routen muessten. Entweder IdObj bekommt also ein Flag
					 * fuer die Richtung, oder hier erfolgt ein Aufruf von
					 * ProbingAnswer(ido). Diese Methode wuerde wieder zu v zurueckrouten.
					 * wenn es bei v angekommen ist, gilt v->value = ido->value und
					 * da wir dort auf v.0 zugreifen koennen, haben wir auch eine
					 * identity von v.0 und koennen die kante erzeugen
					 */
				}
				else if(ido->value > value/2){
					//schicke nach links
					if(left!=NULL){
						left->out->call(List::Probing, ido);
					}
					else{
						//probe ist steckengeblieben: erzeuge kante
					}
				}
				//der aktuelle knoten ist v.0 => beende probing
			}
		}
	}

}
