/*
 *   Janus, a portable, unified and lightweight interface for mitm
 *   applications over the traffic directed to the default gateway.
 *
 *   Copyright (C) 2011 evilaliv3 <giovanni.pellerano@evilaliv3.org>
 *                      vecna <vecna@delirandom.net>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config_macros.h"
#include "janus.h"
#include "string.h"

static void execOSCmd(char *buf, size_t bufsize, const char *format, ...)
{
    char cmd[CONST_JANUS_BUFSIZE] = {0};
    FILE *stream = NULL;

    va_list arguments;
    va_start(arguments, format);
    vsnprintf(cmd, sizeof (cmd), format, arguments);
    va_end(arguments);

    printf("executing cmd: [%s]\n", cmd);

    stream = popen(cmd, "r");
    if (stream != NULL)
    {
        if (buf != NULL)
        {
            memset(buf, 0, bufsize);

            if (fgets(buf, bufsize, stream) != NULL)
            {
                const size_t len = strlen(buf);

                if (len && buf[len - 1] == '\n')
                    buf[len - 1] = '\0';
            }
        }

        pclose(stream);
    }
}

static void (*bindCmd(struct cmd_sw cmd[]))(char* buf, size_t bufsize)
{
    char test[CONST_JANUS_BUFSIZE] = {0};

    uint8_t i = 0;
    while (cmd[i].cmd_test != NULL)
    {
        execOSCmd(test, sizeof (test), "which %s", cmd[i].cmd_test);
        if (strlen(test))
        {
            printf("binding executed using: %s\n", cmd[i].cmd_test);
            return cmd[i].cmd_ex;
        }

        ++i;
    }

    return NULL;
}

static void cmd0_route(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "route -n | sed -n 's/^\\(0.0.0.0\\).* \\([0-9.]\\{7,15\\}\\) .*\\(0.0.0.0\\).*UG.* \\(.*\\)$/\\4/p'");
}

static void cmd1_ifconfig(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "ifconfig %s | sed -n 's/.*inet addr:\\([0-9.]\\+\\) .*$/\\1/p'", str_map[STR_NET_IF]);
}

static void cmd2_ifconfig(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "ifconfig %s | sed -n 's/^.* HWaddr \\([a-fA-F0-9:]\\{17,17\\}\\).*$/\\1/p'", str_map[STR_NET_IF]);
}

static void cmd3_ifconfig(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "ifconfig -a %s | sed -n 's/^.* MTU:\\([0-9]*\\) .*$/\\1/p'", str_map[STR_NET_IF]);
}

static void cmd4_route(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "route -n | sed -n 's/^\\(0.0.0.0\\).* \\([0-9.]\\{7,15\\}\\) .*\\(0.0.0.0\\).*UG.* %s$/\\2/p'", str_map[STR_NET_IF]);
}

static void cmd5_arp(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "arp -ni %s %s | sed -n 's/^.*\\([a-fA-F0-9:]\\{17,17\\}\\).*$/\\1/p'", str_map[STR_NET_IF], str_map[STR_GW_IP]);
}

static void cmd5_arping(char* buf, size_t bufsize)
{
    execOSCmd(buf, bufsize, "arping -f -I %s %s | sed -n 's/^.*\\([a-fA-F0-9:]\\{16,16\\}\\)\\].*$/0\\1/p'", str_map[STR_NET_IF], str_map[STR_GW_IP]);
}

static void cmd6_route(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "route add default gw %s dev %s", str_map[STR_GW_IP], str_map[STR_NET_IF]);
}

static void cmd7_route(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "route del default gw %s dev %s", str_map[STR_GW_IP], str_map[STR_NET_IF]);
}

static void cmd8_route(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "route add default gw %s dev %s", str_map[STR_TUN_IP], str_map[STR_TUN_IF]);
}

static void cmd9_route(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "route del default gw %s dev %s", str_map[STR_TUN_IP], str_map[STR_TUN_IF]);
}

static void cmd10_iptables(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "iptables -A INPUT -i %s -m mac --mac-source %s -j DROP", str_map[STR_NET_IF], str_map[STR_GW_MAC]);
}

static void cmd11_iptables(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "iptables -D INPUT -i %s -m mac --mac-source %s -j DROP", str_map[STR_NET_IF], str_map[STR_GW_MAC]);
}

static void cmd12_iptables(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "iptables -A FORWARD -i %s -m mac --mac-source %s -j DROP", str_map[STR_NET_IF], str_map[STR_GW_MAC]);
}

static void cmd13_iptables(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "iptables -D FORWARD -i %s -m mac --mac-source %s -j DROP", str_map[STR_NET_IF], str_map[STR_GW_MAC]);
}

static void cmd14_iptables(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "iptables -A POSTROUTING -o %s -t nat -j MASQUERADE", str_map[STR_TUN_IF]);
}

static void cmd15_iptables(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "iptables -D POSTROUTING -o %s -t nat -j MASQUERADE", str_map[STR_TUN_IF]);
}

static void cmd16_ifconfig(char* buf, size_t bufsize)
{
    execOSCmd(NULL, 0, "ifconfig %s %s pointopoint %s mtu %s", str_map[STR_TUN_IF], str_map[STR_NET_IP], str_map[STR_TUN_IP], str_map[STR_TUN_MTU]);
}

static struct cmd_sw cmd0_sw[] = {
    {"route", cmd0_route},
    {NULL, NULL}
};

static struct cmd_sw cmd1_sw[] = {
    {"ifconfig", cmd1_ifconfig},
    {NULL, NULL}
};

static struct cmd_sw cmd2_sw[] = {
    {"ifconfig", cmd2_ifconfig},
    {NULL, NULL}
};

static struct cmd_sw cmd3_sw[] = {
    {"ifconfig", cmd3_ifconfig},
    {NULL, NULL}
};

static struct cmd_sw cmd4_sw[] = {
    {"route", cmd4_route},
    {NULL, NULL}
};

static struct cmd_sw cmd5_sw[] = {
    {"arp", cmd5_arp},
    {"arping", cmd5_arping},
    {NULL, NULL}
};

static struct cmd_sw cmd6_sw[] = {
    {"route", cmd6_route},
    {NULL, NULL}
};

static struct cmd_sw cmd7_sw[] = {
    {"route", cmd7_route},
    {NULL, NULL}
};

static struct cmd_sw cmd8_sw[] = {
    {"route", cmd8_route},
    {NULL, NULL}
};

static struct cmd_sw cmd9_sw[] = {
    {"route", cmd9_route},
    {NULL, NULL}
};

static struct cmd_sw cmd10_sw[] = {
    {"iptables", cmd10_iptables},
    {NULL, NULL}
};

static struct cmd_sw cmd11_sw[] = {
    {"iptables", cmd11_iptables},
    {NULL, NULL}
};

static struct cmd_sw cmd12_sw[] = {
    {"iptables", cmd12_iptables},
    {NULL, NULL}
};

static struct cmd_sw cmd13_sw[] = {
    {"iptables", cmd13_iptables},
    {NULL, NULL}
};

static struct cmd_sw cmd14_sw[] = {
    {"iptables", cmd14_iptables},
    {NULL, NULL}
};

static struct cmd_sw cmd15_sw[] = {
    {"iptables", cmd15_iptables},
    {NULL, NULL}
};

static struct cmd_sw cmd16_sw[] = {
    {"ifconfig", cmd16_ifconfig},
    {NULL, NULL}
};

static struct
{
    struct cmd_sw *sw;
} cmd_sw_table[] = {
    {cmd0_sw},
    {cmd1_sw},
    {cmd2_sw},
    {cmd3_sw},
    {cmd4_sw},
    {cmd5_sw},
    {cmd6_sw},
    {cmd7_sw},
    {cmd8_sw},
    {cmd9_sw},
    {cmd10_sw},
    {cmd11_sw},
    {cmd12_sw},
    {cmd13_sw},
    {cmd14_sw},
    {cmd15_sw},
    {cmd16_sw},
    {0}
};

static void (*cmd[COMMANDS_NUM])(char* buf, size_t bufsize);

static void bindCmds(void)
{
    char test[CONST_JANUS_BUFSIZE] = {0};

    uint8_t i;

    printf("checking sed command presence\n");
    execOSCmd(test, sizeof (test), "which sed");
    if (!strlen(test))
        runtime_exception("unable to find sed command");

    for (i = 0; cmd_sw_table[i].sw != NULL; ++i)
    {
        printf("binding cmd %u to a system command\n", i);
        cmd[i] = bindCmd(cmd_sw_table[i].sw);
        if (cmd[i] == NULL)
            runtime_exception("unable to bind cmd %u to a system command", i);
    }
}
