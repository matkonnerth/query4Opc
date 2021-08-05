(1) all Objects

MATCH (objs:Object) return objs;

(2) all variables

MATCH (vars:Variables) return vars;

(3) objects of type ServerType

MATCH (objs:Object)-[:HASTYPEDEFINITION]->(:ObjectType{NodeId:"ServerType"})