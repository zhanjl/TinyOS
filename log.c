#include "log.h"
#include "fs.h"
#include "file.h"
#include "monitor.h"
#include "mem.h"
#include "buf.h"
struct log log;

static void recover_from_log(void);
static void commit();
void initlog(void)
{
    if (sizeof(struct logheader) >= BSIZE)
        PANIC("initlog: too big header");

    struct superblock sb;
    readsb(ROOTDEV, &sb);
    log.start = sb.size - sb.nlog;
    log.size = sb.nlog;
    log.dev = ROOTDEV;
    recover_from_log();
}

static void install_trans(void)
{
    int tail;

    for (tail = 0; tail < log.lh.n; tail++)
    {
        struct buf *lbuf = bread(log.dev, log.start+tail+1);
        struct buf *dbuf = bread(log.dev, log.lh.sector[tail]);

        memcpy(dbuf->data, lbuf->data, BSIZE);
        bwrite(dbuf);
        brelse(lbuf);
        brelse(dbuf);
    }
}

static void read_head(void)
{
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *lh = (struct logheader*)(buf->data);
    int i;
    log.lh.n = lh->n;
    for (i = 0; i < log.lh.n; i++)
    {
        log.lh.sector[i] = lh->sector[i];
    }
    brelse(buf);
}


static void write_head(void)
{
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *hb = (struct logheader *)(buf->data);
    int i;
    hb->n = log.lh.n;
    for (i = 0; i < log.lh.n; i++)
    {
        hb->sector[i] = log.lh.sector[i];
    }

    bwrite(buf);
    brelse(buf);
}


static void recover_from_log(void)
{
    read_head();
    install_trans();
    log.lh.n = 0;
    write_head();
}

void begin_op(void)
{

    while (1) 
    {
        if (log.committing)
        {
            continue;
        } 
        else if (log.lh.n + (log.outstanding+1)*MAXOPBLOCKS > LOGSIZE)
        {
            continue;
        }
        else
        {
            log.outstanding += 1;
            break; 
        }
    }
}

void end_op(void)
{
    int     do_commint = 0;

    log.outstanding -= 1;
    if (log.committing)
        PANIC("log.committing");
    if (log.outstanding == 0)
    {
        do_commint = 1;
        log.committing = 1;
    }

    if (do_commint)
    {
        commit();
        log.committing = 0;
    }
}

static void write_log(void)
{
    int tail;
    for (tail = 0; tail < log.lh.n; tail++)
    {
        struct buf *to = bread(log.dev, log.start+tail+1);
        struct buf *from = bread(log.dev, log.lh.sector[tail]);
        memcpy(to->data, from->data, BSIZE);
        bwrite(to);
        brelse(from);
        brelse(to);
    }
}

static void commit()
{
    if (log.lh.n > 0)
    {
        write_log();
        write_head();
        install_trans();
        log.lh.n = 0;
        write_head();
    }
}

void
log_write(struct buf *b)
{
    int i;

    if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)
        PANIC("too big a transaction");
    if (log.outstanding < 1)
       PANIC("log_write outside of trans");
    
     for (i = 0; i < log.lh.n; i++) {
        if (log.lh.sector[i] == b->sector)   // log absorbtion
            break;
     }
     log.lh.sector[i] = b->sector;
     if (i == log.lh.n)
        log.lh.n++;
     b->flags |= BUF_DIRTY; // prevent eviction
}
