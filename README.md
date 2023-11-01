# My Own Database
## Relational Database Management System made from scratch in C
> Note: This was made for learning purposes and it is not meant to be used in real projects

# Features
- Insertion with fixed structure

| name | email | height |
|------|-------|--------|
|text(32)|text(20)|integer|

- Selection by position
- Selection by name
- Hash Indices

# Usage
1. Create required files `touch <your-db-name>.db  <your-db-name>.idx`
2. Compile `make`
3. Run `./main <your-db-name>`
## Commands
```
insert <name> <email> <height>
select <position>
select <name>
exit
```
