
How to run:
===========

**To test:**

    make run_test

**To run DB:**

    make run

If readline wrapper is installed. You could do:

    make
    rlwrap ./basedb


Some notes on current status and futures ideas
===============================================

==> One index should be created while creating the table itself.
==> If the index was changed in between:
    * Additional time would be taken to index the whole db.
    * All the duplicate keys will be removed and only the latest one will remain.
==> File name is as the tablename.


==> Only one index allowed now.

==> Table index will act like a write through cache. When ever the cache is updated the physical copy is also updated along with it.

==> Have to load index if any of the operations are done.

==> Free up spaces.

==> Modification can be done using inserting a second time.

==> Change the hash table to balanced tree.

==> Test with large number of insertions

==> To be implemented:
        createdb DBNAME;
        destroydb DBNAME;
        opendb DBNAME;
        closedb;
        destroy RELATION_NAME;
        load RELATION_NAME from FILENAME;
        project into RELATION_NAME from RELATION_NAME ( ATTR_NAME [ , ATTR_NAME ]* );
        join into RELATION_NAME ( RELATION_NAME . ATTR_NAME, RELATION_NAME . ATTR_NAME );
 

==> Implement drop index DONE
==> Test deletion DONE



==> check if the input is correct
==> implement support for float


Acknowledgment
===============

This project wa inspired from MINIREL. The frontend of MINIREL was taken into this project. 

MINIREL was originally conceived as an instructional RDBMS design student project by Professor David DeWitt of the University of Wisconsin.

I have downloaded the minirel code from the web page of [Associate Prof. Murali Krishnan K](http://athena.nitc.ac.in/~kmurali/dbms/minirel.html)
