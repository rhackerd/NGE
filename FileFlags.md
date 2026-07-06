<!--
 Copyright 2026 rhacker
 
 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
     https://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
-->

# Tags
Because this is my old project and i am trying to rewrite it i will use tags.

Every source file (.h, .cpp, .slang) should be signed directly under it's license header to keep track to it's refactoring state.

## What is a file tag
A file tag is a comment at the top of the source file. It provides a visual indicator so i can see and track the status of files.

## Tags
Can be written as #TAG, but to be more explicit can be written as #TAG@VER, so
```cpp
// TODO: modernize
// ========================================
// Metadata
//
// State    : #modern@nge2
// Origin   : @nge2
//
// Desc     : Short description about the specific file
// Info     : More additional info.
// ========================================
```
says that this file was already modern enough in version NGE2.


### Per File
#### File states
State of the file.
| Tag | Phase Name | Description |
| :---| :--------- | :---------- |
| `#legacy` | **Legacy Unmanaged** | Either not used or legacy code which has the most priority. |
| `#modernizing` | **As the name suggests** | As the name suggests in this phase the file is being rewritten to be more modern and up to spec |
| `#modern` | **Modern rewrite** | The file is fully up to spec, mainly for wrappers but big files like `device.h` needs to be managed |
| `#managed` | **Actively being updated** | Indicates that the file is already completed and `#modern` but is still being changed and in development |
#### Origin
Origin from where the file came, for example `legacy` when the file came from NGE2, and `new` if the file is new for splitting and new functionility thus for NGE3.
| Tag | Short Desc | Long Desc |
| :---| :--------- | :---------- |
| `@ngeX` | **Came from NGE2** | should only use `@ngeX` not `#...@ngeX`, the X says the version |
### Per Impl
#### State
The state of the implementation. The state of an implementation can either be `#legacy` or `#modern` not in the middle, a single PR should transition between `#legacy` and `#modern`
| Tag | Short Desc | Long Desc |
| :---| :--------- | :---------- |
| `#legacy` | **Came from NGE2** | Legacy . |
| `#modern` | **New addition** | As the name suggests in this phase the file is being rewritten to be more modern and up to spec |

## Why
Because this project is used for my games and for the development of my engine.

And to start taking this project more seriously.

## How to modernize
First before writing any code, there should be a written lists of #TODO's so programmers are aware what to exactly fix and not add any new functionality and go off-course for example changing implementation.

After that step the modernizing process should start. And only after if the file is in the modern state the file can be updated, which should then have the #managed tag.

## Usage
This tag is required on all source files on top of the file, but i recommend to please also describe functions and in which state they are in, because `#modernizing` tag itself does not tell in which state the process is.

## FAQ
**Q: What happens when all files are either modern or managed ?**
A: the library is ready for the 2.0.0 release.


## Template

```cpp
// TODO: modernize
// ========================================
// Metadata
//
// State    : #legacy@nge3
// Origin   : @nge2
//
// Desc     : Short description about the specific file
// Info     : More additional info.
// ========================================
```