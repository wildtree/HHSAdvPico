//
// ZRules
//

#ifndef ZRULE_H
#define ZRULE_H

#include <SD.h>
#include <random>
#include <zuserdata.h>
#include <queue>

class ZCommand
{
public:
    enum Command {
        Nop = 0,
        Message,
        Sound,
        Dialogue,
        GameOver,
    };
    ZCommand() : _cmd(Nop), _operand(0) {}
    ZCommand(Command c, uint8_t o) : _cmd(c), _operand(o) {}

    Command getCommand() const { return _cmd; }
    uint8_t getOperand() const { return _operand; }
protected:
    Command _cmd;
    uint8_t _operand;    
};

class ZCore
{
protected:
    union {
        uint8_t b[8];
        struct
        {
            uint8_t _map_id, _map_view, _cmd_id, _obj_id, _dlg_res, _random, _dlg_ok, _dlg_mesg;
        } attr;
    } _stat;
    std::queue<ZCommand> _queue;
public:
    ZCore();

    uint8_t mapId() const { return _stat.attr._map_id; }
    void mapId(uint8_t v) { _stat.attr._map_id = v; }
    uint8_t mapView() const { return _stat.attr._map_view; }
    void mapView(uint8_t v) { _stat.attr._map_view = v; }
    uint8_t cmdId() const { return _stat.attr._cmd_id; }
    void cmdId(uint8_t v) { _stat.attr._cmd_id = v; }
    uint8_t objId() const { return _stat.attr._obj_id; }
    void objId(uint8_t v) { _stat.attr._obj_id = v; }
    uint8_t dlgRes() const { return _stat.attr._dlg_res; }
    void dlgRes(uint8_t v) { _stat.attr._dlg_res = v; }
    uint8_t rand() ;
    void rand(uint8_t v);
    uint8_t dlgOk() const { return _stat.attr._dlg_ok; }
    void dlgOk(uint8_t v) { _stat.attr._dlg_ok = v; }
    uint8_t dlgMsg() const { return _stat.attr._dlg_mesg; }
    void setValue(int id, uint8_t v) { _stat.b[id] = v; }
    uint8_t getValue(int id) const { return _stat.b[id]; }
    void unpack(const uint8_t buf[]);
    uint8_t *pack() const;
    static constexpr int packed_size = 8;

    bool is_empty() { return _queue.empty(); }
    void push(ZCommand::Command c, uint8_t o) { _queue.push(ZCommand(c,o));}
    ZCommand pop();
    void flush();
};

class ZRuleBlock
{
protected:
    union {
        uint8_t _b[4];
        struct {
            uint8_t _pad:4;
            uint8_t _op:3;
            uint8_t _act:1;
            uint8_t _id:5;
            uint8_t _typ:3;
            uint8_t _body_offset:8;
            uint8_t _body_value:8;
        } bits;
    } _block;
    typedef union {
        uint8_t _v;
        struct {
            uint8_t _body_id:5;
            uint8_t _body_type:3;
        } bits;
    } body_offset_t;
public:
    ZRuleBlock();
    ZRuleBlock(const uint8_t b[]);
    ZRuleBlock(const ZRuleBlock &x);

    ZRuleBlock& operator =(const ZRuleBlock &x);

    uint8_t action() const { return (uint8_t)(_block.bits._act & 0xff); }
    uint8_t op() const { return (uint8_t)(_block.bits._op & 0xff); }
    uint8_t type() const { return (uint8_t)(_block.bits._typ & 0xff); }
    uint8_t id() const { return (uint8_t)(_block.bits._id & 0xff); }
    uint8_t offset() const { return (uint8_t)(_block.bits._body_offset & 0xff); }
    uint8_t value() const { return (uint8_t)(_block.bits._body_value & 0xff); }
    uint8_t body_type() const;
    uint8_t body_id() const;

    uint8_t getOp1(ZCore *c, ZUserData *u) const;
    uint8_t getOp2(ZCore *p, ZUserData *u) const;

    bool actCmp() const { return _block.bits._act == actComp; }
    bool actAct() const { return _block.bits._act == actAction; }
    bool doCmp(ZCore *c, ZUserData *u) const;
    bool doAct(ZCore *c, ZUserData *u) const;

    static const int actComp = 0;
    static const int actAction = 1;

    static const int cmpNop = 0;
    static const int cmpEq = 1;
    static const int cmpNe = 2;
    static const int cmpGt = 3;
    static const int cmpGe = 4;
    static const int cmpLt = 5;
    static const int cmpLe = 6;

    static const int actNop = 0;
    static const int actMove = 1;
    static const int actAsgn = 2;
    static const int actMesg = 3;
    static const int actDlog = 4;
    static const int actLook = 5;
    static const int actSnd = 6;
    static const int actOver = 7;

    static const int typNone = 0;
    static const int typFact = 1;
    static const int typPlace = 2;
    static const int typSystem = 3;
    static const int typVector = 4;
};

class ZRuleBase
{
protected:
    ZRuleBlock *_rules;
    int _map_id, _cmd_id, _obj_id;

    const int end_of_rule_mark = 0xff;
    const int rule_block_length = file_block_size / 4 - 1;
public:
    ZRuleBase(void);
    ZRuleBase(const uint8_t *buf);
    ZRuleBase(const ZRuleBase &x);
    ~ZRuleBase();

    ZRuleBase operator =(const ZRuleBase &x);

    bool end_of_rules(void) const { return _map_id == end_of_rule_mark; }
    int mapId(void) const { return _map_id; }
    int cmdId(void) const { return _cmd_id; }
    int objId(void) const { return _obj_id; }

    bool about(int map_id, int cmd_id, int obj_id) const;
    bool about(const ZCore *core) const { return about(core->mapId(), core->cmdId(), core->objId()); }
    bool run(ZCore *core, ZUserData *user) const;

    static const int max_rules = 256;
    static const int file_block_size = 96;
};

class ZRulesIterator;
class ZRules
{
protected:
    ZRuleBase _rules[ZRuleBase::max_rules];
public:
    ZRules(const String &f);
    
    ZRulesIterator begin() const;
    ZRulesIterator end() const;
    bool run(ZCore *core, ZUserData *user);

    friend class ZRulesIterator;
};

class ZRulesIterator
{
protected:
    const ZRules *_r;
    int _ptr;
public:
    ZRulesIterator();
    explicit ZRulesIterator(const ZRules *r, int p);
    ZRulesIterator(const ZRulesIterator &x);
    ZRulesIterator &operator =(const ZRulesIterator &x);
    bool operator ==(const ZRulesIterator &x) const;
    bool operator !=(const ZRulesIterator &x) const;
    const ZRuleBase &operator *();
    ZRulesIterator &operator ++();
    ZRulesIterator operator ++(int);
};

#endif /* ZRULE_H */