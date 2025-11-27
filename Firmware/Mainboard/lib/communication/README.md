# Team Thermocline Communications

Super super simple format.

Want the whole team to be able to use this!

Structure is *just plain json*

every command takes this form:

```json
{
  "type": "command_name",    // required: the actual command name (e.g. "ping", "get_status")
  "data": {                  // optional: payload data as a JSON object
    // ...command-specific arguments as needed
    // So like
    // "value1": 123,
    // "flag": true,
    // "note": "some optional message"
  }
}
```

Examples:
- Ping command: `{"type": "ping", "data": {}}`
- Get status: `{"type": "get_status", "data": {}}`
- Command with args: `{"type": "set_value", "data": {"value": 42}}`

and replies take this form:

```json
{
    "type": "reply",
    "data": {
        "success": true,   // Or false if you're unlucky
    },
}
```

