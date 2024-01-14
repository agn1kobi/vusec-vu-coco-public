from util import ASTTransformer
from fennec_ast import Type, Operator, VarDef, ArrayDef, Assignment, Modification, \
        If, Block, VarUse, BinaryOp, IntConst, Return, While


class Desugarer(ASTTransformer):
    def __init__(self):
        self.varcache_stack = [{}]

    def makevar(self, name):
        # Generate a variable starting with an underscore (which is not allowed
        # in the language itself, so should be unique). To make the variable
        # unique, add a counter value if there are multiple generated variables
        # of the same name within the current scope.
        # A variable can be tagged as 'ssa' which means it is only assigned once
        # at its definition.
        name = '_' + name
        varcache = self.varcache_stack[-1]
        occurrences = varcache.setdefault(name, 0)
        varcache[name] += 1
        return name if not occurrences else name + str(occurrences + 1)

    def visitFunDef(self, node):
        self.varcache_stack.append({})
        self.visit_children(node)
        self.varcache_stack.pop()

    def visitModification(self, m):
        # from: lhs op= rhs
        # to:   lhs = lhs op rhs
        self.visit_children(m)
        return Assignment(m.ref, BinaryOp(m.ref, m.op, m.value)).at(m)
    
    def visitFor(self, node):
        # from: for(int type ID = INTCONST to INTCONST) {body}
        # to: {
        # int type ID = INTCONST
        # while(ID < INTCONST) {
        # body
        # ID = ID + 1
        # }
        #}
        self.visit_children(node)
        if isinstance(node.entry, str):
            entry = VarUse(node.entry)
        init_ass = VarDef(Type.get('int'), str(entry), node.lexpr).at(node)
        cng = Assignment(entry, BinaryOp(entry, Operator('+'), IntConst(1))).at(node)
        while_body = Block([node.body, cng]).at(node)
        while_cond = BinaryOp(entry, Operator("<"), node.rexpr).at(node)
        while_stat = While(while_cond, while_body).at(node)
        while_stat.make_desugar(entry)
        final_block = Block([init_ass, while_stat]).at(node)
        return final_block
