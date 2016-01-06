from simple_page_rank import SimplePageRank

"""
This class implements the pagerank algorithm with
backwards edges as described in the second part of
the project.
"""
class BackedgesPageRank(SimplePageRank):

    """
    The implementation of __init__ and compute_pagerank should
    still be the same as SimplePageRank.
    You are free to override them if you so desire, but the signatures
    must remain the same.
    """

    """
    This time you will be responsible for implementing the initialization
    as well.
    Think about what additional information your data structure needs
    compared to the old case to compute weight transfers from pressing
    the 'back' button.
    """
    @staticmethod
    def initialize_nodes(input_rdd):
        # YOUR CODE HERE
        # The pattern that this solution uses is to keep track of
        # (node, (weight, targets, old_weight)) for each iteration.
        # When calculating the score for the next iteration, you
        # know that 10% of the score you sent out from the previous
        # iteration will get sent back.
        # return input_rdd.filter(lambda x: False)
        def emit_edges(line):
            # ignore blank lines and comments
            if len(line) == 0 or line[0] == "#":
                return []
            # get the source and target labels
            source, target = tuple(map(int, line.split()))
            # emit the edge
            edge = (source, frozenset([target]))
            # also emit "empty" edges to catch nodes that do not have any
            # other node leading into them, but we still want in our list of nodes
            self_source = (source, frozenset())
            self_target = (target, frozenset())
            return [edge, self_source, self_target]

        # collects all outgoing target nodes for a given source node
        def reduce_edges(e1, e2):
            return e1 | e2

        # sets the weight of every node to 0, and formats the output to the
        # specified format of (source (weight, targets))
        def initialize_weights((source, targets)):
            return (source, (1.0, targets, -1.0))

        nodes = input_rdd\
                .flatMap(emit_edges)\
                .reduceByKey(reduce_edges)\
                .map(initialize_weights)
        return nodes

    """
    You will also implement update_weights and format_output from scratch.
    You may find the distribute and collect pattern from SimplePageRank
    to be suitable, but you are free to do whatever you want as long
    as it results in the correct output.
    """
    @staticmethod
    def update_weights(nodes, num_nodes):
        # YOUR CODE HERE
        # return nodes.filter(lambda x: False)
        def distribute_weights((node, (weight, targets, old_w))):
            # YOUR CODE HERE
            # ret_arr = []
            #
            # ret_arr.append((node, ( weight * .05 + .1, targets)))
            # if len(targets) == 0:
            #     distrib_weight = 0.85 * weight / (num_nodes - 1)
            #     for target in range(num_nodes):
            #         if target is not node:
            #             ret_arr.append((target, (distrib_weight, [])))
            # else:
            #     distrib_weight = 0.85 * weight / len(targets)
            #     for target in targets:
            #         ret_arr.append((target, (distrib_weight, [])))
            # return ret_arr
            # ret_arr = []
            # curr_val = weight * 0.05 + 0.1 * dict[node]
            # # ret_arr.append((node, (node, weight * 0.05 + 0.1 * dict[node],targets )))
            # dict.pop(node)
            # if len(targets) == 0:
            #     distrib_weight = 0.85 * weight / (num_nodes - 1)
            #     for target in range(num_nodes):
            #         if target is not node:
            #             pagerank = distrib_weight + 0.1 * dict.get(target, 0.0)
            #             dict.pop(target, None)
            #             ret_arr.append((target, (node, pagerank, [])))
            # else:
            #     distrib_weight = 0.85 * weight / len(targets)
            #     for target in targets:
            #         if target is node:
            #             curr_val += distrib_weight
            #         else:
            #             pagerank = distrib_weight + 0.1 * dict.get(target, 0.0)
            #             dict.pop(target, None)
            #             ret_arr.append((target, (node, pagerank, [])))
            # for key in dict.keys():
            #     ret_arr.append((key,(node, dict[key] * 0.1, [])))
            #
            # ret_arr.append((node, (node, curr_val,targets )))
            # return ret_arr

            ret_arr = []

            if old_w < 0:
                ret_arr.append((node, (0.15, targets, 1)))
            else:
                ret_arr.append((node, (0.05 * weight + 0.1 * old_w, targets, weight)))
            if len(targets) == 0:
                distrib_weight = 0.85 * weight / (num_nodes - 1)
                for target in range(num_nodes):
                    if target is not node:
                        ret_arr.append((target, (distrib_weight, [], 0.0)))
            else:
                distrib_weight = 0.85 * weight / len(targets)
                for target in targets:
                    ret_arr.append((target, (distrib_weight, [], 0.0)))
            return ret_arr


        def collect_weights((node, values)):
            # YOUR CODE HERE
            values_arr = []
            page_sum = 0.0
            old_sum = 0.0

            for (page_score, arr, w) in values:
                page_sum += page_score
                values_arr.extend(arr)
                old_sum += w

            # return (node,(page_sum, values_arr))
            return (node, (page_sum, values_arr, old_sum))

        return nodes\
                .flatMap(distribute_weights)\
                .groupByKey()\
                .map(collect_weights)

    @staticmethod
    def format_output(nodes):
        # YOUR CODE HERE
        # return nodes.filter(lambda x: False)
        # def format_output(nodes):
        #     return nodes\
        #             .map(lambda (node, (weight, targets)): (weight, node))\
        #             .sortByKey(ascending = False)\
        #             .map(lambda (weight, node): (node, weight))

        return nodes\
                .map(lambda (node, (weight, targets, dict)): (weight, node))\
                .sortByKey(ascending = False)\
                .map(lambda (weight, node): (node,weight))
