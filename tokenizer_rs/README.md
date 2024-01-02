# Tokenizer

**Small query language for text transformation**
Example of usage can be found in _tests_ directory.
For example taking _in_1.txt_ as input:

```
 import { applyDmgPercent } from '../../../calc' import { makeHit } from '../../../hit';
import { ShipSystem } from '../../types';
import empoweredBlastSystemMeta from './empowered-blast.json';

export const EmpoweredBlast = (): ShipSystem => ({
  ...empoweredBlastSystemMeta,
  type: 'battle',
  targets: 'single',
  charges: 5,
  maxCharges: 5,
  use({ sourceName, sourceStats, aliveTargets }) {
    const target = aliveTargets[0];
    sourceStats.hitChance -= 20;
    applyDmgPercent(sourceStats, 300);
    makeHit(sourceName, sourceStats, target);
  },
});

```

and applying _cmds_1.txt_:

```
@0 spl any :,'"
@0 trim
@0{targets,charges,maxCharges} and_next_only
@0 to_json

```

result will be:

```
{
  "targets": "single",
  "charges": 5,
  "maxCharges": 5
}
```

## Explaining

At any given point of time, we have state that can be `list` or `string` (`list item`)
Every `list item` can have other `lists` or `strings`.

Every command have a format of: `arg1 function arg2 arg3 argN`.
All things separated with `space`.
`arg1` is special argument that works as a selector of list:

- `@0`: select main list
- `@n`: select `n-th` list item of main list
- `@n{one,two,three}`: select strings `one`, `two`, `three` from `n-th` list item (pattern matching)

Now, using prev. example:

1. `@0 spl any :,'"` - split main list into strings if separator any of `:`, `,`, `'`, `"`
2. `@0 trim` - trim all strings in main list
3. `@0{targets,charges,maxCharges} and_next_only` - select `targets,charges,maxCharges` from main list and leave only them and every next item from them
4. `@0 to_json` - convert main list to json

## How To Run

`cargo build -r && ./target/release/tokenizer_rs`

### OR

`cargo run`
