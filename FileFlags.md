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
| Tag | Phase Name | Description |
| :---| :--------- | :---------- |
| `#legacy` | **Legacy Unmanaged** | Either not used or legacy code which has the most priority. |
| `#modernizing` | **As the name suggests** | As the name suggests in this phase the file is being rewritten to be more modern and up to spec |
| `#modern` | **Modern rewrite** | The file is fully up to spec, mainly for wrappers but big files like `device.h` needs to be managed |
| `#managed` | **Actively being updated** | This file is fully up to spec and is being updated 

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


