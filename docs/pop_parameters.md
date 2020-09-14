# POP Parameters {#popparameters}

<table>
<tr>
    <th>Parameter</th>
    <th>Default Value</th>
    <th>Description</th>
    <th>Recommended Value</th>
</tr>
<tr>
    <td>`POP Payout Delay`</td>
    <td>50 blocks</td>
    <td>If block 30 is endorsed, and delay=50, then payout for this block will be in block 30+50=80</td>
    <td>TODO</td>
</tr>
<tr>
    <td>`Keystone Interval`</td>
    <td>5 blocks</td>
    <td>Number of blocks inside single keystone interval. If 5, then blocks with heights 5, 6, 7, 8, 9 are in same keystone interval, etc.</td>
    <td>TODO</td>
</tr>
<tr>
    <td>`Endorsement Settlement Interval`</td>
    <td>50 blocks</td>
    <td>Validity window for endorsements. If endorsed block height E=51, and this settlement interval=100, then endorsement can be mined in any block in range (51..151], on any chain which contains endorsed block. If current tip is 152, this endorsement becomes invalid and is removed from POP MemPool.</td>
    <td>TODO</td>
</tr>

</table>

TODO (write down recommendations on picking parameters)
