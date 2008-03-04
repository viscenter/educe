function neighb_set=find_n_layer_neighb(node_set, neighb_array, layer_num)
%neighb_set=find_n_layer_neighb(node_set, neighb_array, layer_num)
neighb_set=node_set;
for i=1:layer_num
    n_set=find_neighb(neighb_set, neighb_array);
    neighb_set=[neighb_set n_set];
end