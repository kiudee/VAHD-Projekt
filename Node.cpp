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
    node0 = NULL;
    node1 = NULL;
    in = new Relay;
    // Connect to supervisor or real node:
    if (isReal) {
        IdPair *idp = new IdPair(new IdObj(id, new Identity(in)),
                         new IdObj(id, new Identity(in)));
        parent->call(Supervisor::SetLink, idp);

        //node0 = new NodeRelay...
        //node1 = new NodeRelay...
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

void Node::checkDead(Relay **side) {
    if (side != NULL && outdeg(side->out) == 0) {
        delete *side->out;
        delete *side;
        *side = NULL;
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
    checkDead(&left);
    checkDead(&right);

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
    BuildDeBruijn(id);
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
 * There is an example for a left probing in the comments. The single steps are ordered by their temporal appearance.
 */
Action Node::Probing(Probe *ido){
	if(isReal){
		if(ido==NULL){
			//If no left neighbor is set, set it to node0 and probing is done, else send probe to the left neighbor
			if(left==NULL){
				tempido = new IdObj(node0->id, new Identity(node0->in));
				left = new NodeRelay(tempido);
			}else{
				//(1) send probe to left introducing the first phase (indicating by parameter 0)
				tempido = new Probe(id, new Identity(in), 0);
				left->out->call(Node::Probing, tempido);
			}
			//If no right neighbor is set, set it to node1 and probing is done, else send the probe to the right neighbor
			if(right==NULL){
				tempido = new IdObj(node1->id, new Identity(node1->in));
				right = new NodeRelay(tempido);
			}else{
				tempido = new Probe(id, new Identity(in), 0);
				right->out->call(Node::Probing, tempido);
			}
		}
		else{
			//probe reaches v.0 or v.1. finish probing.
			if(value==ido->value/2 || value==1+ido->value/2){
				delete ido;
				return;
			}

			/*probe came from the left, so we're searching for ido->value/2
			because we're on a real node, change the direction flag and
			send probe to node0*/
			if(ido->value > value){
				//(3) we are on the real node w, so we switch the phase flag indicating, that we are on the left side of v.0.
				tempido = new Probe(ido->value, extractIdentity(ido->out), 1);
				delete ido;
				node0->out->call(Node::Probing, tempido);
			}
			/*probe came from the right, so we're searching for 1+ido->value/2
			because we're on a real node, change the direction flag and
			send probe to node1*/
			if(ido->value < value){

				tempido = new Probe(ido->value, extractIdentity(ido->out), 1);
				delete ido;
				node1->out->call(Node::Probing, tempido);
			}

		}


	}else{
		if(ido!=NULL){
			//probe came from the left
			if(ido->value > value){
				//probe reaches v.0. finish probing
				if(value == ido->value/2){
					delete ido;
					return;
				}
				//probe hit no real node yet, so send it to left
				if(ido->phase == 0){
					if(left!=NULL){
						//(2) send probe to the left
						left->out->call(List::Probing, ido);
						return;
					}
				}
				//probe hit real node, so send it to right
				else if(ido->phase == 1){
					if(value < ido->value/2){
						if(right!=NULL){
							//(4) send probe to the right
							right->out->call(List::Probing, ido);
							return;
						}
					}
				}
				/*if the probe gets stuck (no left or right neighbor) or
				the id was not found, establish link between v and v.0*/
				//@TODO does this work?
				NodeRelay temprelay = new NodeRelay(ido);
				temprelay->out->call(Node::BuildWeakConnectedComponent, 0);
				delete temprelay;
			}
			if(ido->value < value){

				//probe reaches v.1. finish probing
				if(value == 1+ido->value/2){
					delete ido;
					return;
				}
				if(ido->phase == 0){
					if(right!=NULL){
						right->out->call(List::Probing, ido);
						return;
					}
				}else if(ido->phase == 1){
					if(value > 1+ido->value/2){
						if(left!=NULL){
							left->out->call(List::Probing, ido);
							return;
						}
					}
				}
				NodeRelay temprelay = new NodeRelay(ido);
				temprelay->out->call(Node::BuildWeakConnectedComponent, 1);
				delete temprelay;
			}

		}
	}
}
/**
 * Builds a link from v to v.0 or v.1
 * @author Simon
 * @param 0=link to v.0, 1=link to v.1
 */
Action Node::BuildWeakConnectedComponent(int node){
	if(node==0){
		//envelope left neighbor
		tempido = new IdObj(left->value, extractIdentity(left->out));
		delete left;
		//link to v.0
		left = new NodeRelay(node0->value, new Identity(node0->in));
		//delegate old left neighbor to node0
		left->out->call(Node::BuildList,tempido);
	}
	else if(node == 1){
		tempido = new IdObj(right->value, extractIdentity(right->out));
		delete right;
		right = new NodeRelay(node1->value, new Identity(node1->in));
		right->out->call(Node::BuildList,tempido);
	}
}
