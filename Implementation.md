Source

"Generates" nodes, example is the hierachical visitor.

Filter

Filters nodes on a certain attribute, for example the typedefinition id.
ReferenceFilter

Sink

stores nodes at the end of filtering

Path???
PathFilter?

(:Variable{NodeId:"MyId"}<-[:HasProperty]-(obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"MyId"})

break it down to

intermediateResults = (obj:Object)-[:HasTypeDefinition]->(:ObjectType{NodeId:"MyId"})
Sink = ReferenceFilter( with ReferenceDescriptionMatcher(HasTypeDefinition))