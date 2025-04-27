/*
 * ZRule
 */

#include <zsystem.h>
#include <zrule.h>
#include <zuserdata.h>
#include <vscroll.h>

ZRuleBlock::ZRuleBlock()
{
    memset(_block._b, 0, sizeof(_block));
}

ZRuleBlock::ZRuleBlock(const uint8_t b[])
{
    memcpy(_block._b, b, sizeof(_block));
#if 0
    Serial1.printf("(action, op, type, id, offset, value, body_type, body_id) = (%d, %d, %d, %d, %d, %d, %d, %d) (%02x %02x %02x %02x)\r\n",
        action(), op(), type(), id(), offset(), value(), body_type(), body_id(), b[0], b[1], b[2], b[3]
    );
#endif
}

ZRuleBlock::ZRuleBlock(const ZRuleBlock &x)
{
    memcpy(_block._b, x._block._b, sizeof(_block));
}

ZRuleBlock &
ZRuleBlock::operator =(const ZRuleBlock &x)
{
    memcpy(_block._b, x._block._b, sizeof(_block));
    return *this;
}

uint8_t
ZRuleBlock::body_type() const
{
    body_offset_t tmp;
    tmp._v = offset();
    return tmp.bits._body_type;
}

uint8_t
ZRuleBlock::body_id() const
{
    body_offset_t tmp;
    tmp._v = offset();
    return tmp.bits._body_id;
}

uint8_t
ZRuleBlock::getOp1(ZCore *c, ZUserData *u) const
{
    uint8_t v = 0;
    switch (type())
    {
    case typNone:
        break;
    case typFact:
        v = u->getFact(id());
        break;
    case typPlace:
        v = u->getPlace(id());
        break;
    case typSystem:
        c->rand(0);
        v = c->getValue(id());
        break;
    case typVector:
        v = u->getMap(offset() - 1).get(id());
        break;
    default:
        break;
    }
    return v;
}

uint8_t
ZRuleBlock::getOp2(ZCore *c, ZUserData *u) const
{
    int v = value();
    if (body_type() != typNone && type() != typVector)
    {
        switch(body_type())
        {
        case typFact:
            v = u->getFact(body_id());
            break;
        case typPlace:
            v = u->getPlace(body_id());
            break;
        case typSystem:
            c->rand(0);
            v = c->getValue(body_id());
            break;
        default:
            break;
        }
    }
    return v;
}

bool
ZRuleBlock::doCmp(ZCore *c, ZUserData *u) const
{
    bool ok = false;
    // Serial1.printf("rule block(comp = %d): (type,id) = (%d,%d)\r\n", action(), type(), id());
    uint8_t v1 = getOp1(c, u);
    // Serial1.printf("rule block(comp = %d): body (type,id) = (%d,%d)\r\n", action(), body_type(), body_id());
    uint8_t v2 = getOp2(c, u);
    // Serial1.printf("op: %d (v1, v2) = (%d, %d)\r\n",op(), v1, v2);
    switch(op())
    {
    case cmpEq: ok = (v1 == v2); break;
    case cmpNe: ok = (v1 != v2); break;
    case cmpGt: ok = (v1 > v2); break;
    case cmpGe: ok = (v1 >= v2); break;
    case cmpLt: ok = (v1 < v2); break;
    case cmpLe: ok = (v1 <= v2); break;
    default:
        break;
    }
    return ok;
}

bool
ZRuleBlock::doAct(ZCore *c, ZUserData *u) const
{
    bool ok = false;
    // Serial1.printf("Action: (op, type, value(), op2) = (%d, %d, %d, %d)\r\n",op(), type(), value(), getOp2(c, u));
    switch (op())
    {
    case actMove:
        if (u->getMap(c->mapId() - 1).get(value()) != 0)
        {
            c->mapId(u->getMap(c->mapId() - 1).get(value()));
            ok = true;
        }
        else
        {
            // check teacher
            if (u->getFact(1) == c->mapId() && c->rand() > 85)
            {
                // put message #0xb5 先生につかまった!
                c->push(ZCommand::Message, 0xb5);
                c->push(ZCommand::GameOver, 1);
                u->setFact(1,0);
                // change color to Sepia ???
                // game over
                ok = false;
            }
            else
            {
                // put message #0xb6 移動できない!
                c->push(ZCommand::Message, (uint8_t)0xb6); // 移動できない
                ok = true;
            }
            
        }
        break;
    case actAsgn:
        switch (type())
        {
        case typFact:
            u->setFact(id(),getOp2(c,u));
            break;
        case typPlace:
            u->setPlace(id(),getOp2(c,u));
            break;
        case typSystem:
            c->setValue(id(), (uint8_t)(getOp2(c,u) & 0xff));
            if (id() == 5)
            {
                c->rand(0);
            }
            break;
        case typVector:
            u->getMap(offset() - 1).setLink(id(),getOp2(c,u));
            break;
        default:
            break;
        }
        ok = true;
        break;
    case actMesg:
        // print message;
        c->push(ZCommand::Message, value());
        ok = true;
        break;
    case actDlog:
        // show dialog
        c->push(ZCommand::Dialogue, value());
        ok = true;
        break;
    case actLook:
        if (value() == 0)
        {
            c->mapId(c->mapView()); // back;
            c->mapView(0);
        }
        else
        {
            c->mapView(c->mapId());
            c->mapId(value());
        }
        ok = true;
        break;
    case actSnd:
        // play sound
        c->push(ZCommand::Sound, value());
        ok = true;
        break;
    case actOver:
        switch(value())
        {
        case 0:
            u->setFact(1,0); // teacher has gone
            c->push(ZCommand::Message,0xee);
            c->push(ZCommand::GameOver,1);
            break;
        case 1:
            // color code to sepia
            c->push(ZCommand::Message,0xee);
            c->push(ZCommand::GameOver,2);
            break;
        case 2:
            // game clear
            c->push(ZCommand::Message,0xef);
            c->push(ZCommand::GameOver,3);
            break;
        }
        // game over
        ok = true;
        break;
    default:
        break;
    }
    return ok;
}

ZCore::ZCore()
{
    memset(_stat.b, 0, sizeof(_stat));

}

uint8_t
ZCore::rand() 
{
    rand(0);
    return _stat.attr._random;
}

void
ZCore::rand(uint8_t seed)
{
    if (seed != 0)
    {
        randomSeed(seed);
    }
    _stat.attr._random = random(256);
}

ZCommand
ZCore::pop()
{
    ZCommand tmp = _queue.front();
    _queue.pop();
    return tmp;
}

void
ZCore::flush()
{
    while(!_queue.empty()) _queue.pop();
}

void
ZCore::unpack(const uint8_t buf[])
{
    memcpy(_stat.b, buf, sizeof(_stat));
}

uint8_t *
ZCore::pack() const
{
    return (uint8_t*)_stat.b;
}

ZRuleBase::ZRuleBase(void)
    :_cmd_id(end_of_rule_mark), _obj_id(end_of_rule_mark), _map_id(end_of_rule_mark)
{
    _rules = nullptr;
}
ZRuleBase::ZRuleBase(const uint8_t *buf)
    :_cmd_id(end_of_rule_mark), _obj_id(end_of_rule_mark), _map_id(end_of_rule_mark)
{
    _rules = new ZRuleBlock[rule_block_length];
    uint8_t rb[4];
    _map_id = buf[0] & 0xff;
    _cmd_id = buf[1] & 0xff;
    _obj_id = buf[2] & 0xff;
    // Serial1.printf("Loading (map_id, cmd_id, obj_id) = (%d, %d, %d)\r\n", _map_id, _cmd_id, _obj_id);
    for (int i = 0 ; i < rule_block_length ; i++)
    {
        memcpy(rb, &buf[4*i + 4], 4);
        _rules[i] = ZRuleBlock(rb);
    }
}

ZRuleBase::ZRuleBase(const ZRuleBase &x)
    :_map_id(x._map_id), _cmd_id(x._cmd_id), _obj_id(x._obj_id)
{
    _rules = new ZRuleBlock[rule_block_length];
    for (int i = 0 ; i < rule_block_length ; i++)
    {
        _rules[i] = x._rules[i];
    }
}

ZRuleBase::~ZRuleBase()
{
    if (_rules) delete[] _rules;
}

ZRuleBase
ZRuleBase::operator =(const ZRuleBase &x)
{
    _map_id = x._map_id;
    _cmd_id = x._cmd_id;
    _obj_id = x._obj_id;
    if (_rules != nullptr) delete[] _rules;

    _rules = new ZRuleBlock[rule_block_length];
    for (int i = 0 ; i < rule_block_length ; i++)
    {
        _rules[i] = x._rules[i];
    }
    return *this;
}

bool
ZRuleBase::about(int map_id, int cmd_id, int obj_id) const
{
    //Serial1.printf("(%d, %d) - (%d, %d) - (%d,%d)\r\n", map_id, _map_id, cmd_id, _cmd_id, obj_id, _obj_id);
    return (map_id == _map_id || _map_id == 0)&&(cmd_id == _cmd_id || _cmd_id == 0)&&(obj_id == _obj_id||_obj_id == 0);
}

bool
ZRuleBase::run(ZCore *core, ZUserData *user) const
{
    // Serial1.print("ZRuleBase::run()\r\n");
    if (about(core))
    {
        //Serial1.printf("about (%d,%d,%d)\r\n", core->mapId(), core->cmdId(), core->objId());
        bool cond_ok = true;
        bool act_ok = true;
        int i = 0;
        while (_rules[i].actCmp())
        {
            // Serial1.printf("rule block %d (%08x)\r\n",i, &_rules[i]);
            cond_ok = _rules[i++].doCmp(core, user);
            if (!cond_ok) return false; // condition check failed.
        }
        // Serial1.printf("%d: Cond Ok? %d\r\n", i, cond_ok);
        if (cond_ok) // all clear. go action phase
        {
            while(_rules[i].op() != ZRuleBlock::actNop)
            {
                act_ok = _rules[i++].doAct(core, user) && act_ok;
            }
        }
        // Serial1.printf("%d: Act Ok? %d\r\n", i, act_ok);
        return true;
    }
    // Serial1.print("run() done.\r\n");
    return false;
}

ZRules::ZRules(const String &file)
{
    File f = SD.open(file);
    uint8_t *buf = new uint8_t [ZRuleBase::file_block_size];
    int i = 0;
    while (f.available())
    {
        f.readBytes((char*)buf, ZRuleBase::file_block_size);
        _rules[i++] = ZRuleBase(buf);
    }
    f.close();
    delete[] buf;
}


ZRulesIterator
ZRules::begin() const
{
    return ZRulesIterator(this, 0);
}

ZRulesIterator
ZRules::end() const
{
    int p = 0;
    for (p = 0 ; !_rules[p].end_of_rules() ; p++) {}
    return ZRulesIterator(this, p);
}

ZRulesIterator::ZRulesIterator() : _ptr(0), _r(nullptr) {}
ZRulesIterator::ZRulesIterator(const ZRulesIterator &x) : _ptr(x._ptr), _r(x._r) {}

ZRulesIterator::ZRulesIterator(const ZRules *r, int p) : _ptr(p), _r(r) {}

ZRulesIterator&
ZRulesIterator::operator =(const ZRulesIterator &x)
{
    _ptr = x._ptr;
    _r = x._r;
    return *this;
}

ZRulesIterator &
ZRulesIterator::operator ++()
{
    _ptr++;
    return *this;
}

ZRulesIterator
ZRulesIterator::operator ++(int)
{
    ZRulesIterator tmp = *this;
    ++*this;
    return tmp;
}

const ZRuleBase &
ZRulesIterator::operator *()
{
    return _r->_rules[_ptr];
}

bool
ZRulesIterator::operator ==(const ZRulesIterator &x) const
{
    return (_r == x._r && _ptr== x._ptr);
}

bool
ZRulesIterator::operator !=(const ZRulesIterator &x) const
{
    return !this->operator==(x);
}