%% -------------------------------------------------------------------
%% Copyright (c) 2007-2012 Basho Technologies, Inc.  All Rights Reserved.
%%
%% This file is provided to you under the Apache License,
%% Version 2.0 (the "License"); you may not use this file
%% except in compliance with the License.  You may obtain
%% a copy of the License at
%%
%%   http://www.apache.org/licenses/LICENSE-2.0
%%
%% Unless required by applicable law or agreed to in writing,
%% software distributed under the License is distributed on an
%% "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
%% KIND, either express or implied.  See the License for the
%% specific language governing permissions and limitations
%% under the License.
%%
%% -------------------------------------------------------------------
-module(erasuerl).
-export([new/4, encode/2, decode/4]).
-on_load(init/0).

-include_lib("eunit/include/eunit.hrl").


-define(nif_stub, nif_stub_error(?LINE)).
nif_stub_error(Line) ->
    erlang:nif_error({nif_not_loaded,module,?MODULE,line,Line}).

init() ->
    PrivDir = case code:priv_dir(?MODULE) of
                  {error, bad_name} ->
                      EbinDir = filename:dirname(code:which(?MODULE)),
                      AppPath = filename:dirname(EbinDir),
                      filename:join(AppPath, "priv");
                  Path ->
                      Path
               end,
     erlang:load_nif(filename:join(PrivDir, ?MODULE), 0).

-spec new(pos_integer(), pos_integer(), pos_integer(), pos_integer()) -> {ok, binary()}.
new(_K, _M, _W, _PacketSize) ->
    ?nif_stub.

encode(_EC, _Bin) ->
    ?nif_stub.

decode(_EC, _Meta, _KList, _MList) ->
    ?nif_stub.

no_erasures_test() ->
    {ok, EC} = erasuerl:new(11, 5, 4, 64),    
    {ok, Bin} = file:read_file("/usr/share/dict/words"),    
    {MD, KBins, MBins} = erasuerl:encode(EC, Bin), 
    ?assertEqual({error, no_erasures}, erasuerl:decode(EC, MD, KBins, MBins)).

too_many_erasures_test() ->
    {ok, EC} = erasuerl:new(5, 3, 4, 64),    
    {ok, Bin} = file:read_file("/usr/share/dict/words"),    
    {MD, KBins, MBins} = erasuerl:encode(EC, Bin), 
    KBins2 = [<<>>, <<>>, <<>>, <<>>, hd(tl(KBins))],
    ?assertEqual({error, unrecoverable}, erasuerl:decode(EC, MD, KBins2, MBins)).

simple_data_erasure_test() ->
    {ok, EC} = erasuerl:new(11, 5, 4, 64),    
    {ok, Bin} = file:read_file("/usr/share/dict/words"),    
    {MD, KBins, MBins} = erasuerl:encode(EC, Bin), 
    KBins2 = [<<>>|tl(KBins)],
    ?assertEqual(Bin, iolist_to_binary(erasuerl:decode(EC, MD, KBins2, MBins))).

simple_coding_erasure_test() ->
    {ok, EC} = erasuerl:new(11, 5, 4, 64),    
    {ok, Bin} = file:read_file("/usr/share/dict/words"),    
    {MD, KBins, MBins} = erasuerl:encode(EC, Bin),    
    MBins2 = [<<>>|tl(MBins)],    
    ?assertEqual(Bin, iolist_to_binary(erasuerl:decode(EC, MD, KBins, MBins2))).

bolth_test() ->   
    {ok, EC} = erasuerl:new(10, 4, 4, 64),    
    {ok, Bin} = file:read_file("/usr/share/dict/words"),    
    {MD, KBins, MBins} = erasuerl:encode(EC, Bin),    
    KBins2 = [<<>>|tl(KBins)],
    MBins2 = [<<>>|tl(MBins)],    
    ?assertEqual(Bin, iolist_to_binary(erasuerl:decode(EC, MD, KBins2, MBins2))).    
