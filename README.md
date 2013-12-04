# `eraserl` - Cauchy-based Reed Solomon Codes for Erlang

Warning
-------
This is still alpha, laptop ware.  Only tested on OSX Mavericks. 


Example
-------

```
simple_data_erasure_test() ->
    {ok, EC} = eraserl:new(11, 5, 4, 64),    
    {ok, Bin} = file:read_file("/usr/share/dict/words"),    
    {MD, KBins, MBins} = eraserl:encode(EC, Bin), 
    KBins2 = [<<>>|tl(KBins)],
    ?assertEqual(Bin, iolist_to_binary(eraserl:decode(EC, MD, KBins2, MBins))).
    
```

